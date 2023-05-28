/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Simon Kuenzer <simon.kuenzer@neclab.eu>
 *          Cristian Vijelie <cristianvijelie@gmail.com>
 *
 *
 * Copyright (c) 2017, NEC Europe Ltd., NEC Corporation. All rights reserved.
 * Copyright (c) 2020, University Politehnica of Bucharest. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __UK_ISR_STRING_H__
#define __UK_ISR_STRING_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

void __attribute__((isr_safe)) *memcpy_isr(void *dst, const void *src, size_t len);
void __attribute__((isr_safe)) *memset_isr(void *ptr, int val, size_t len);
void __attribute__((isr_safe)) *memchr_isr(const void *ptr, int val, size_t len);
void __attribute__((isr_safe)) *memrchr_isr(const void *m, int c, size_t n);
int __attribute__((isr_safe)) memcmp_isr(const void *ptr1, const void *ptr2, size_t len);
void __attribute__((isr_safe)) *memmove_isr(void *dst, const void *src, size_t len);

char __attribute__((isr_safe)) *strncpy_isr(char *dst, const char *src, size_t len);
char __attribute__((isr_safe)) *strcpy_isr(char *dst, const char *src);
size_t __attribute__((isr_safe)) strlcpy_isr(char *d, const char *s, size_t n);
size_t __attribute__((isr_safe)) strlcat_isr(char *d, const char *s, size_t n);
size_t __attribute__((isr_safe)) strnlen_isr(const char *str, size_t maxlen);
size_t __attribute__((isr_safe)) strlen_isr(const char *str);
char __attribute__((isr_safe)) *strchrnul_isr(const char *s, int c);
char __attribute__((isr_safe)) *strchr_isr(const char *str, int c);
char __attribute__((isr_safe)) *strrchr_isr(const char *s, int c);
int __attribute__((isr_safe)) strncmp_isr(const char *str1, const char *str2, size_t len);
int __attribute__((isr_safe)) strcmp_isr(const char *str1, const char *str2);
size_t __attribute__((isr_safe)) strcspn_isr(const char *s, const char *c);
size_t __attribute__((isr_safe)) strspn_isr(const char *s, const char *c);
char __attribute__((isr_safe)) *strtok_isr(char *restrict s, const char *restrict sep, char **restrict p);

#ifdef __cplusplus
}
#endif

#endif /* __UK_ISR_STRING_H__ */
