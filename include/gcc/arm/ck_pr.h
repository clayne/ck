/*
 * Copyright 2009-2013 Samy Al Bahra.
 * Copyright 2013 Olivier Houchard.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _CK_PR_ARM_H
#define _CK_PR_ARM_H

#ifndef _CK_PR_H
#error Do not include this file directly, use ck_pr.h
#endif

#include <ck_cc.h>
#include <ck_md.h>

/*
 * The following represent supported atomic operations.
 * These operations may be emulated.
 */
#include "ck_f_pr.h"

/*
 * Minimum interface requirement met.
 */
#define CK_F_PR

CK_CC_INLINE static void
ck_pr_stall(void)
{

	__asm__ __volatile__("" ::: "memory");
	return;
}

#if defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__)
#define __CK_ISB __asm __volatile("isb" : : "r" (0) : "memory")
#define _CK_DMB __asm __volatile("dmb" : : "r" (0) : "memory")
#define _CK_DSB __asm __volatile("dsb" : : "r" (0) : "memory")
/* FreeBSD's toolchain doesn't accept dmb st, so use the opcode instead */
#ifdef __FreeBSD__
#define _CK_DMB_ST __asm __volatile(".word 0xf57ff05e" : : "r" (0) : "memory")
#else
#define _CK_DMB_ST __asm __volatile("dmb st" : : "r" (0) : "memory")
#endif /* __FreeBSD__ */
#else
/* armv6 doesn't have dsb/dmb/isb, and no way to wait only for stores */
#define _CK_ISB \
    __asm __volatile("mcr p15, 0, %0, c7, c5, 4" : : "r" (0) : "memory")
#define _CK_DSB \
    __asm __volatile("mcr p15, 0, %0, c7, c10, 4" : : "r" (0) : "memory")
#define _CK_DMB  \
    __asm __volatile("mcr p15, 0, %0, c7, c10, 5" : : "r" (0) : "memory")
#define _CK_DMB_ST _CK_DMB
#endif

#define CK_PR_FENCE(T, I)				\
	CK_CC_INLINE static void			\
	ck_pr_fence_strict_##T(void)			\
	{						\
		I;					\
	}

CK_PR_FENCE(atomic, _CK_DMB_ST)
CK_PR_FENCE(atomic_store, _CK_DMB_ST)
CK_PR_FENCE(atomic_load, _CK_DMB_ST)
CK_PR_FENCE(store_atomic, _CK_DMB_ST)
CK_PR_FENCE(load_atomic, _CK_DMB)
CK_PR_FENCE(store, _CK_DMB_ST)
CK_PR_FENCE(store_load, _CK_DMB_ST)
CK_PR_FENCE(load, _CK_DMB)
CK_PR_FENCE(load_store, _CK_DMB)
CK_PR_FENCE(memory, _CK_DMB)
CK_PR_FENCE(acquire, _CK_DMB)
CK_PR_FENCE(release, _CK_DMB)

#undef CK_PR_FENCE

#undef _CK_ISB
#undef _CK_DSB
#undef _CK_DMB
#undef _CK_DMB_ST

#define CK_PR_LOAD(S, M, T, C, I)				\
	CK_CC_INLINE static T					\
	ck_pr_load_##S(const M *target)				\
	{							\
		long r = 0;					\
		__asm__ __volatile__(I " %0, [%1];"		\
					: "=r" (r)		\
					: "r"  (target)		\
					: "memory");		\
		return ((T)r);					\
	}

CK_PR_LOAD(ptr, void, void *, uint32_t, "ldr")

#define CK_PR_LOAD_S(S, T, I) CK_PR_LOAD(S, T, T, T, I)

CK_PR_LOAD_S(32, uint32_t, "ldr")
CK_PR_LOAD_S(16, uint16_t, "ldrh")
CK_PR_LOAD_S(8, uint8_t, "ldrb")
CK_PR_LOAD_S(uint, unsigned int, "ldr")
CK_PR_LOAD_S(int, int, "ldr")
CK_PR_LOAD_S(short, short, "ldrh")
CK_PR_LOAD_S(char, char, "ldrb")

#undef CK_PR_LOAD_S
#undef CK_PR_LOAD

CK_CC_INLINE static uint64_t
ck_pr_load_64(const uint64_t *target)
{
	register uint64_t ret asm("r0");

	__asm __volatile("ldrd %0, [%1]" : "+r" (ret)
	    				 : "r" (target) 
					 : "memory", "cc");
	return (ret);
}

#define CK_PR_STORE(S, M, T, C, I)				\
	CK_CC_INLINE static void				\
	ck_pr_store_##S(M *target, T v)				\
	{							\
		__asm__ __volatile__(I " %1, [%0]"		\
					:			\
					: "r" (target),		\
					  "r" (v)		\
					: "memory");		\
		return;						\
	}

CK_PR_STORE(ptr, void, const void *, uint32_t, "str")

#define CK_PR_STORE_S(S, T, I) CK_PR_STORE(S, T, T, T, I)

CK_PR_STORE_S(32, uint32_t, "str")
CK_PR_STORE_S(16, uint16_t, "strh")
CK_PR_STORE_S(8, uint8_t, "strb")
CK_PR_STORE_S(uint, unsigned int, "str")
CK_PR_STORE_S(int, int, "str")
CK_PR_STORE_S(short, short, "strh")
CK_PR_STORE_S(char, char, "strb")

#undef CK_PR_STORE_S
#undef CK_PR_STORE

CK_CC_INLINE static void
ck_pr_store_64(const uint64_t *target, uint64_t value)
{
	register uint64_t tmp asm("r0") = value;

	__asm __volatile("strd %0, [%1]"
				:
				: "r" (tmp), "r" (target)
				: "memory", "cc");
}

CK_CC_INLINE static bool
ck_pr_cas_64_value(uint64_t *target, uint64_t compare, uint64_t set, uint64_t *value)
{
	register uint64_t __compare asm("r0") = compare;
	register uint64_t __set asm("r2") = set;

	__asm__ __volatile__("1:"
			     "ldrexd r4, [%3];"
			     "cmp    r4, r0;"
			     "ittt eq;"
			     "cmpeq  r5, r1;"
			     "strexdeq r6, r2, [%3];"
			     "cmpeq  r6, #1;"
			     "beq 1b;"
			     "strd r4, [%0];"
				: "+r" (value)
				: "r" (__compare), "r" (__set) ,
				  "r"(target)
				: "memory", "cc", "r4", "r5", "r6");
	return (*value == compare);
}

CK_CC_INLINE static bool
ck_pr_cas_ptr_2_value(void *target, void *compare, void *set, void *value)
{
	uint32_t *_compare = compare;
	uint32_t *_set = set;
	uint64_t __compare = ((uint64_t)_compare[0]) | ((uint64_t)_compare[1] << 32);
	uint64_t __set = ((uint64_t)_set[0]) | ((uint64_t)_set[1] << 32);

	return (ck_pr_cas_64_value(target, __compare, __set, value));
}


CK_CC_INLINE static bool
ck_pr_cas_64(uint64_t *target, uint64_t compare, uint64_t set)
{
	register uint64_t __compare asm("r0") = compare;
	register uint64_t __set asm("r2") = set;
	int ret;

	__asm__ __volatile__("1:"
			     "mov %0, #0;"
			     "ldrexd r4, [%3];"
			     "cmp    r4, r0;"
			     "itttt eq;"
			     "cmpeq  r5, r1;"
			     "strexdeq r6, r2, [%3];"
			     "moveq %0, #1;"
			     "cmpeq  r6, #1;"
			     "beq 1b;"
			     : "=&r" (ret)
			     : "r" (__compare), "r" (__set) ,
			       "r"(target)
			     : "memory", "cc", "r4", "r5", "r6");

	return (ret);
}

CK_CC_INLINE static bool
ck_pr_cas_ptr_2(void *target, void *compare, void *set)
{
	uint32_t *_compare = compare;
	uint32_t *_set = set;
	uint64_t __compare = ((uint64_t)_compare[0]) | ((uint64_t)_compare[1] << 32);
	uint64_t __set = ((uint64_t)_set[0]) | ((uint64_t)_set[1] << 32);
	return (ck_pr_cas_64(target, __compare, __set));
}

CK_CC_INLINE static bool
ck_pr_cas_ptr_value(void *target, void *compare, void *set, void *value)
{
	void *previous, *tmp;
	__asm__ __volatile__("1:"
			     "ldrex %0, [%2];"
			     "cmp   %0, %4;"
			     "itt eq;"
			     "strexeq %1, %3, [%2];"
			     "cmpeq   %1, #1;"
			     "beq   1b;"
			  	: "=&r" (previous),
				  "=&r" (tmp)
		  		: "r"   (target),
				  "r"   (set),
				  "r"   (compare)	
				: "memory", "cc");
	*(void **)value = previous;
	return (previous == compare);
}

CK_CC_INLINE static bool
ck_pr_cas_ptr(void *target, void *compare, void *set)
{
	void *previous, *tmp;
	__asm__ __volatile__("1:"
			     "ldrex %0, [%2];"
			     "cmp   %0, %4;"
			     "itt eq;"
			     "strexeq %1, %3, [%2];"
			     "cmpeq   %1, #1;"
			     "beq   1b;"
			  	: "=&r" (previous),
				  "=&r" (tmp)
		  		: "r"   (target),
				  "r"   (set),
				  "r"   (compare)	
				: "memory", "cc");
	return (previous == compare);
}

#define CK_PR_CAS(N, T, W)						\
	CK_CC_INLINE static bool					\
	ck_pr_cas_##N##_value(T *target, T compare, T set, T *value)	\
	{								\
		T previous = 0, tmp = 0;				\
		__asm__ __volatile__("1:"				\
				     "ldrex" W " %0, [%2];"		\
				     "cmp   %0, %4;"			\
				     "itt eq;"				\
				     "strex" W "eq %1, %3, [%2];"	\
		    		     "cmpeq   %1, #1;"			\
				     "beq   1b;"			\
			/* 						\
			 * Using "+&" instead of "=&" to avoid bogus	\
			 * clang warnings.				\
			 */						\
					: "+&r" (previous),		\
		    			  "+&r" (tmp)			\
					: "r"   (target),		\
					  "r"   (set),			\
					  "r"   (compare)		\
					: "memory", "cc");		\
		*value = previous; 					\
		return (previous == compare);				\
	}								\
	CK_CC_INLINE static bool					\
	ck_pr_cas_##N(T *target, T compare, T set)			\
	{								\
		T previous = 0, tmp = 0;				\
		__asm__ __volatile__("1:"				\
				     "ldrex" W " %0, [%2];"		\
				     "cmp   %0, %4;"			\
				     "itt eq;"				\
				     "strex" W "eq %1, %3, [%2];"	\
				     "cmpeq   %1, #1;"			\
				     "beq   1b;"			\
					: "+&r" (previous),		\
		    			  "+&r" (tmp)			\
					: "r"   (target),		\
					  "r"   (set),			\
					  "r"   (compare)		\
					: "memory", "cc");		\
		return (previous == compare);				\
	}

CK_PR_CAS(32, uint32_t, "")
CK_PR_CAS(uint, unsigned int, "")
CK_PR_CAS(int, int, "")
CK_PR_CAS(16, uint16_t, "h")
CK_PR_CAS(8, uint8_t, "b")
CK_PR_CAS(short, short, "h")
CK_PR_CAS(char, char, "b")


#undef CK_PR_CAS_S
#undef CK_PR_CAS

#define CK_PR_FAS(N, M, T, W)					\
	CK_CC_INLINE static T					\
	ck_pr_fas_##N(M *target, T v)				\
	{							\
		T previous = 0;					\
		T tmp = 0;					\
		__asm__ __volatile__("1:"			\
				     "ldrex" W " %0, [%2];"	\
				     "strex" W " %1, %3, [%2];"	\
		    		     "cmp %1, #0;"		\
				     "bne 1b;"			\
					: "+&r" (previous),	\
		    			  "+&r" (tmp) 		\
					: "r"   (target),	\
					  "r"   (v)		\
					: "memory", "cc");	\
		return (previous);				\
	}

CK_PR_FAS(32, uint32_t, uint32_t, "")
CK_PR_FAS(ptr, void, void *, "")
CK_PR_FAS(int, int, int, "")
CK_PR_FAS(uint, unsigned int, unsigned int, "")
CK_PR_FAS(16, uint16_t, uint16_t, "h")
CK_PR_FAS(8, uint8_t, uint8_t, "b")
CK_PR_FAS(short, short, short, "h")
CK_PR_FAS(char, char, char, "b")


#undef CK_PR_FAS

#define CK_PR_UNARY(O, N, M, T, I, W)				\
	CK_CC_INLINE static void				\
	ck_pr_##O##_##N(M *target)				\
	{							\
		T previous = 0;					\
		T tmp = 0;					\
		__asm__ __volatile__("1:"			\
				     "ldrex" W " %0, [%2];"	\
				      I ";"			\
				     "strex" W " %1, %0, [%2];"	\
		    		     "cmp   %1, #0;"		\
				     "bne   1b;"		\
					: "+&r" (previous),	\
		    			  "+&r" (tmp)		\
					: "r"   (target)	\
					: "memory", "cc");	\
		return;						\
	}

CK_PR_UNARY(inc, ptr, void, void *, "add %0, %0, #1", "")
CK_PR_UNARY(dec, ptr, void, void *, "sub %0, %0, #1", "")
CK_PR_UNARY(not, ptr, void, void *, "mvn %0, %0", "")
CK_PR_UNARY(neg, ptr, void, void *, "neg %0, %0", "")

#define CK_PR_UNARY_S(S, T, W)					\
	CK_PR_UNARY(inc, S, T, T, "add %0, %0, #1", W)		\
	CK_PR_UNARY(dec, S, T, T, "sub %0, %0, #1", W)		\
	CK_PR_UNARY(not, S, T, T, "mvn %0, %0", W)		\
	CK_PR_UNARY(neg, S, T, T, "neg %0, %0", W)		\

CK_PR_UNARY_S(32, uint32_t, "")
CK_PR_UNARY_S(uint, unsigned int, "")
CK_PR_UNARY_S(int, int, "")
CK_PR_UNARY_S(16, uint16_t, "h")
CK_PR_UNARY_S(8, uint8_t, "b")
CK_PR_UNARY_S(short, short, "h")
CK_PR_UNARY_S(char, char, "b")

#undef CK_PR_UNARY_S
#undef CK_PR_UNARY

#define CK_PR_BINARY(O, N, M, T, I, W)				\
	CK_CC_INLINE static void				\
	ck_pr_##O##_##N(M *target, T delta)			\
	{							\
		T previous = 0;					\
		T tmp = 0;					\
		__asm__ __volatile__("1:"			\
				     "ldrex" W " %0, [%2];"	\
				      I " %0, %0, %3;"		\
				     "strex" W " %1, %0, [%2];"	\
		    		     "cmp %1, #0;"		\
				     "bne 1b;"			\
					: "+&r" (previous),	\
		    			  "+&r" (tmp)		\
					: "r"   (target),	\
					  "r"   (delta)		\
					: "memory", "cc");	\
		return;						\
	}

CK_PR_BINARY(and, ptr, void, uintptr_t, "and", "")
CK_PR_BINARY(add, ptr, void, uintptr_t, "add", "")
CK_PR_BINARY(or, ptr, void, uintptr_t, "orr", "")
CK_PR_BINARY(sub, ptr, void, uintptr_t, "sub", "")
CK_PR_BINARY(xor, ptr, void, uintptr_t, "eor", "")

#define CK_PR_BINARY_S(S, T, W)			\
	CK_PR_BINARY(and, S, T, T, "and", W)	\
	CK_PR_BINARY(add, S, T, T, "add", W)	\
	CK_PR_BINARY(or, S, T, T, "orr", W)	\
	CK_PR_BINARY(sub, S, T, T, "sub", W)	\
	CK_PR_BINARY(xor, S, T, T, "eor", W)

CK_PR_BINARY_S(32, uint32_t, "")
CK_PR_BINARY_S(uint, unsigned int, "")
CK_PR_BINARY_S(int, int, "")
CK_PR_BINARY_S(16, uint16_t, "h")
CK_PR_BINARY_S(8, uint8_t, "b")
CK_PR_BINARY_S(short, short, "h")
CK_PR_BINARY_S(char, char, "b")

#undef CK_PR_BINARY_S
#undef CK_PR_BINARY

CK_CC_INLINE static void *
ck_pr_faa_ptr(void *target, uintptr_t delta)
{
	uintptr_t previous, r, tmp;

	__asm__ __volatile__("1:"
			     "ldrex %0, [%3];"
			     "add %1, %4, %0;"
			     "strex %2, %1, [%3];"
			     "cmp %2, #0;"
			     "bne  1b;"
				: "=&r" (previous),
				  "=&r" (r),
				  "=&r" (tmp)
				: "r"   (target),
				  "r"   (delta)
				: "memory", "cc");

	return (void *)(previous);
}

#define CK_PR_FAA(S, T, W)						\
	CK_CC_INLINE static T						\
	ck_pr_faa_##S(T *target, T delta)				\
	{								\
		T previous = 0, r = 0, tmp = 0;				\
		__asm__ __volatile__("1:"				\
				     "ldrex" W " %0, [%3];"		\
				     "add %1, %4, %0;"			\
				     "strex" W " %2, %1, [%3];"		\
		    		     "cmp %2, #0;"			\
				     "bne  1b;"				\
					: "+&r" (previous),		\
					  "+&r" (r),			\
		    			  "+&r" (tmp)			\
					: "r"   (target),		\
					  "r"   (delta)			\
					: "memory", "cc");		\
		return (previous);					\
	}

CK_PR_FAA(32, uint32_t, "")
CK_PR_FAA(uint, unsigned int, "")
CK_PR_FAA(int, int, "")
CK_PR_FAA(16, uint16_t, "h")
CK_PR_FAA(8, uint8_t, "b")
CK_PR_FAA(short, short, "h")
CK_PR_FAA(char, char, "b")

#undef CK_PR_FAA

#endif /* _CK_PR_ARM_H */

