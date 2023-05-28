#include <stdio.h>

#include <gcc-plugin.h>
#include <tree.h>
#include <tree-iterator.h>
#include <gimple-expr.h>
#include <diagnostic-core.h>
#include <plugin-version.h>
#include <print-tree.h>
#include <stringpool.h>
#include <attribs.h>

#include "tree-pass.h"
#include "context.h"
#include "function.h"
#include "tree.h"
#include "tree-ssa-alias.h"
#include "internal-fn.h"
#include "is-a.h"
#include "predict.h"
#include "basic-block.h"
#include "gimple-expr.h"
#include "gimple.h"
#include "gimple-pretty-print.h"
#include "gimple-iterator.h"
#include "gimple-walk.h"

int plugin_is_GPL_compatible; // must be defined & exported for the plugin to be loaded

static struct attribute_spec isr_safe_attr =
{
        "isr_safe",
        0,
        0,
        false,
        false,
        false,
        NULL,  // No need for a handling function
};

static void register_attributes(void* event_data, void* data)
{
        register_attribute(&isr_safe_attr);
}

static bool fn_is_isr_safe(tree fndecl)
{
        return lookup_attribute("isr_safe", DECL_ATTRIBUTES(fndecl)) != NULL_TREE;
}

static bool fn_is_isr_safe_helper(tree fndecl)
{
        const char *name = get_name(fndecl);

        if (fn_is_isr_safe(fndecl))
                return true;

        if (DECL_SECTION_NAME(fndecl)
                && !strcmp(DECL_SECTION_NAME(fndecl), "isr"))
                return true;

        return !strcmp(name, "__builtin_expect") // ctx.h, ctx.c, ectx.c
                || !strcmp(name, "ukplat_terminate")
                || !strcmp(name, "_uk_printk")
                || !strcmp(name, "uk_printd")
                || !strcmp(name, "memset")
                || !strcmp(name, "UK_INIT_LIST_HEAD") // fallocbuddy.c
                || !strcmp(name, "uk_list_add_tail")
                || !strcmp(name, "uk_list_del")
                || !strcmp(name, "uk_list_add")
                || !strcmp(name, "uk_list_empty")
                || !strcmp(name, "ukarch_fls")
                || !strcmp(name, "ukarch_ffsl")
                || !strcmp(name, "ukarch_flsl")
                || !strcmp(name, "ukarch_ffs")
                || !strcmp(name, "_do_mbox_recv") // mbox_isr.c
                || !strcmp(name, "_do_mbox_post")
                || !strcmp(name, "uk_semaphore_down_try")
                || !strcmp(name, "nf_handle_trap") // maccess.c
                || !strcmp(name, "nf_excpttab_get_cont_ip")
                || !strcmp(name, "pgarch_pt_add_mem") // paging.c
                || !strcmp(name, "pg_page_mapx")
                || !strcmp(name, "pg_ffree")
                || !strcmp(name, "pg_falloc")
                || !strcmp(name, "uk_asmdumpk") // traps.c
                || !strcmp(name, "vmem_vma_unmap") // vmem.c
                || !strcmp(name, "vmem_vma_destroy")
                || !strcmp(name, "vmem_vma_split")
                || !strcmp(name, "vmem_vma_do_try_merge_with_next")
                || !strcmp(name, "vmem_mapx_populate")
                || !strcmp(name, "vmem_vma_advise")
                || !strcmp(name, "vmem_vma_set_attr")
                || !strcmp(name, "uk_malloc") //vma_dma.c
                || !strcmp(name, "vfscore_get_file") //vma_file.c
                || !strcmp(name, "fdrop")
                || !strcmp(name, "fhold")
                || !strcmp(name, "uk_free")
                || !strcmp(name, "ukplat_tlsp_get") // uk_syscall_binary.c
                || !strcmp(name, "ukplat_tlsp_set")
                || !strcmp(name, "uk_thread_current")
                || !strcmp(name, "uk_syscall6_r")
                ;
}

static bool fn_has_target(tree fndecl)
{
        return lookup_attribute("target", DECL_ATTRIBUTES(fndecl))
                != NULL_TREE;
}

static bool fn_has_additional_section(tree fndecl)
{
        return lookup_attribute("section", DECL_ATTRIBUTES(fndecl))
                != NULL_TREE;
}

static void update_isr_safe_function_attributes(tree *fndecl)
{
        tree new_attributes = make_attribute(
                "target",
                "no-avx,no-mmx,no-sse",
                DECL_ATTRIBUTES(*fndecl)
        );

        decl_attributes(fndecl, new_attributes, 0, NULL);

        new_attributes = make_attribute(
                "section",
                "isr",
                DECL_ATTRIBUTES(*fndecl)
        );
        decl_attributes(fndecl, new_attributes, 0, NULL);
}

static void mark_isr_safe_function(tree *fndecl)
{
        tree new_attributes = make_attribute(
                "isr_safe",
                NULL,
                DECL_ATTRIBUTES(*fndecl)
        );
        decl_attributes(fndecl, new_attributes, 0, NULL);
}

static void inspect_call_statement(gimple *g)
{
        if (!is_gimple_call(g))
                return;

        location_t l = gimple_location(g);
        enum gimple_code code = gimple_code(g);

        // fprintf(stderr, "Statement of type: %s at %s:%d\n",
        //         gimple_code_name[code],
        //         LOCATION_FILE(l),
        //         LOCATION_LINE(l));

        tree called_fn_decl = gimple_call_fndecl(g);

        // debug_tree(called_fn_decl);

        // if (!fn_is_isr_safe(called_fn_decl)) {
        if (!fn_is_isr_safe_helper(called_fn_decl)) {
                warning_at(gimple_location(g), 0,
                        "isr-safe function called non-isr-safe function\n");
        }
}

namespace
{
        const pass_data mark_isr_safe_pass_data = 
        {
                GIMPLE_PASS,
                "mark_isr_safe_pass",   /* name */
                OPTGROUP_NONE,          /* optinfo_flags */
                TV_NONE,                /* tv_id */
                PROP_gimple_any,        /* properties_required */
                0,                      /* properties_provided */
                0,                      /* properties_destroyed */
                0,                      /* todo_flags_start */
                0                       /* todo_flags_finish */
        };

        struct mark_isr_safe_pass : gimple_opt_pass
        {
                mark_isr_safe_pass(gcc::context *ctx)
                : gimple_opt_pass(mark_isr_safe_pass_data, ctx)
                {
                }

                virtual unsigned int execute(function *fun) override
                {
                        tree fndecl = fun->decl;

                        if (fn_is_isr_safe(fndecl)) {
                                printf("found: (%s:%d) %s, %p\n",
                                        DECL_SOURCE_FILE(fndecl),
                                        DECL_SOURCE_LINE(fndecl),
                                        get_name(fndecl),
                                        DECL_ATTRIBUTES(fndecl));

                                update_isr_safe_function_attributes(&fndecl);

                                // fun is the current function being called
                                gimple_seq gimple_body = fun->gimple_body;

                                struct walk_stmt_info walk_stmt_info;
                                memset(&walk_stmt_info, 0, sizeof(walk_stmt_info));

                                walk_gimple_seq(gimple_body,
                                        callback_stmt,
                                        callback_op,
                                        &walk_stmt_info);
                        }

                        return 0;
                }

                virtual mark_isr_safe_pass* clone() override
                {
                        // We do not clone ourselves
                        return this;
                }

                private:

                static tree callback_stmt(
                        gimple_stmt_iterator *gsi,
                        bool *handled_all_ops,
                        struct walk_stmt_info *wi)
                {
                        gimple* g = gsi_stmt(*gsi);
                        // debug_gimple_stmt(g);

                        inspect_call_statement(g);

                        return NULL;
                }

                static tree callback_op(tree *t, int *, void *data)
                {
                        return NULL;
                }
        };
}

int plugin_init(
        struct plugin_name_args* plugin_info,
        struct plugin_gcc_version* version)
{
        register_callback(
                plugin_info->base_name,
                PLUGIN_ATTRIBUTES,
                register_attributes,
                NULL
        );

        // Register the phase right after omplower
        struct register_pass_info pass_info;

        // Note that after the cfg is built, fun->gimple_body is not accessible
        // anymore so we run this pass just before the cfg one
        pass_info.pass = new mark_isr_safe_pass(g);
        pass_info.reference_pass_name = "cfg";
        pass_info.ref_pass_instance_number = 1;
        pass_info.pos_op = PASS_POS_INSERT_BEFORE;

        register_callback (plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &pass_info);

        return 0;
}
