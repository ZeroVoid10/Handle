/*******************************************************************************
 * Copyright:		BUPT
 * File Name:		kfifo.h
 * Description:		移植Linux内核kfifo.h 环形队列
 * Author:			ZeroVoid
 * Version:			0.1
 * Data:			2019/11/23 Sat 19:32
 * Encoding:		UTF-8
 *******************************************************************************/
#ifndef __KFIFO_H
#define __KFIFO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "utils.h"

struct __kfifo {
    uint32_t in;
    uint32_t out;
    uint32_t mask;
    uint32_t esize;
    void     *data;
};

// 不知道除了kfifo之外其他东西什么用
#define __STRUCT_KFIFO_COMMON(datatype, recsize, ptrtype) \
    union { \
        struct __kfifo 		kfifo; \
        datatype			*type; \
		const datatype		*const_type; \
		char				(*rectype)[recsize]; \
		ptrtype				*ptr; \
		ptrtype const		*ptr_const; \
    }

#define __STRUCT_KFIFO(type, size, recsize, ptrtype) \
{ \
    __STRUCT_KFIFO_COMMON(type, recsize, ptrtype); \
    type buf[((size < 2) || (size & (size - 1)))? -1:size]; \
}

#define __STRUCT_KFIFO_PTR(type, recsize, ptrtype) \
{ \
	__STRUCT_KFIFO_COMMON(type, recsize, ptrtype); \
	type		buf[0]; \
}

#define STRUCT_KFIFO_PTR(type) \
	struct __STRUCT_KFIFO_PTR(type, 0, type)

#define	__is_kfifo_ptr(fifo) \
	(sizeof(*fifo) == sizeof(STRUCT_KFIFO_PTR(typeof(*(fifo)->type))))

#define STRUCT_KFIFO(type, size) \
    struct __STRUCT_KFIFO(type, size, 0, type)

#define DECLARE_KFIFO(fifo, type, size) STRUCT_KFIFO(type, size) fifo

#define INIT_KFIFO(fifo) \
(void)({ \
    typeof(&(fifo)) __tmp = &(fifo); \
    struct __kfifo *__kfifo = &__tmp->kfifo; \
    __kfifo->in = 0; \
    __kfifo->out = 0; \
    __kfifo->mask = __is_kfifo_ptr(__tmp) ? 0 : ARRAY_SIZE(__tmp->buf) - 1;\
    __kfifo->esize = sizeof(*__tmp->buf); \
    __kfifo->data = __is_kfifo_ptr(__tmp)? NULL: __tmp->buf; \
})

#define DEFINE_KFIFO(fifo, type, size) \
	DECLARE_KFIFO(fifo, type, size) = \
	(typeof(fifo)) { \
		{ \
			{ \
			.in	= 0, \
			.out	= 0, \
            .mask	= __is_kfifo_ptr(&(fifo)) ? \
				  0 : \
				  ARRAY_SIZE((fifo).buf) - 1, \
			.esize	= sizeof(*(fifo).buf), \
			.data	= __is_kfifo_ptr(&(fifo)) ? \
				NULL : \
				(fifo).buf, \
			} \
		} \
	}

/**
#if GCC_VERSION >= 30400
    #define __must_check		__attribute__((warn_unused_result))
#endif
*/
static inline uint32_t __must_check
__kfifo_uint_must_check_helper(uint32_t val)
{
	return val;
}

static inline int32_t __must_check
__kfifo_int_must_check_helper(int32_t val)
{
	return val;
}

#define kfifo_initialized(fifo) ((fifo)->kfifo.mask)

#define kfifo_esize(fifo)	((fifo)->kfifo.esize)

#define kfifo_recsize(fifo)	(sizeof(*(fifo)->rectype))

#define kfifo_size(fifo)	((fifo)->kfifo.mask + 1)

#define kfifo_reset_out(fifo)	\
(void)({ \
	typeof((fifo) + 1) __tmp = (fifo); \
	__tmp->kfifo.out = __tmp->kfifo.in; \
})

#define kfifo_len(fifo) \
({ \
	typeof((fifo) + 1) __tmpl = (fifo); \
	__tmpl->kfifo.in - __tmpl->kfifo.out; \
})

#define	kfifo_is_empty(fifo) \
({ \
	typeof((fifo) + 1) __tmpq = (fifo); \
	__tmpq->kfifo.in == __tmpq->kfifo.out; \
})

#define	kfifo_is_full(fifo) \
({ \
	typeof((fifo) + 1) __tmpq = (fifo); \
	kfifo_len(__tmpq) > __tmpq->kfifo.mask; \
})

#define	kfifo_avail(fifo) \
__kfifo_uint_must_check_helper( \
({ \
	typeof((fifo) + 1) __tmpq = (fifo); \
	const size_t __recsize = sizeof(*__tmpq->rectype); \
	unsigned int __avail = kfifo_size(__tmpq) - kfifo_len(__tmpq); \
	(__recsize) ? ((__avail <= __recsize) ? 0 : \
	__kfifo_max_r(__avail - __recsize, __recsize)) : \
	__avail; \
}) \
)

// remove smp_wmb(); 感觉不需要内存屏障 & recsize, 使用中recsize均为0
// size_t __recsize = sizeof(*__tmp->rectype);
#define	kfifo_put(fifo, val) \
({ \
	typeof((fifo) + 1) __tmp = (fifo); \
	typeof(*__tmp->const_type) __val = (val); \
	uint32_t __ret; \
	struct __kfifo *__kfifo = &__tmp->kfifo; \
    __ret = !kfifo_is_full(__tmp); \
    if (__ret) { \
        (__is_kfifo_ptr(__tmp) ? \
        ((typeof(__tmp->type))__kfifo->data) : \
        (__tmp->buf) \
        )[__kfifo->in & __tmp->kfifo.mask] = \
            *(typeof(__tmp->type))&__val; \
        __kfifo->in++; \
    } \
	__ret; \
})

#define kfifo_get_next_buf(fifo, val) \
({ \
	typeof((fifo) + 1) __tmp = (fifo); \
	uint32_t __ret; \
	struct __kfifo *__kfifo = &__tmp->kfifo; \
    __ret = !kfifo_is_full(__tmp); \
	if (__ret) { \
		val = \
        &(__is_kfifo_ptr(__tmp) ? \
        ((typeof(__tmp->type))__kfifo->data) : \
        (__tmp->buf) \
        )[__kfifo->in & __tmp->kfifo.mask]; \
		__kfifo->in++; \
	} \
	__ret; \
})

// __kfifo_uint_must_check_helper(
#define	kfifo_get(fifo, val) (\
({ \
	typeof((fifo) + 1) __tmp = (fifo); \
	typeof(__tmp->ptr) __val = (val); \
	uint32_t __ret; \
	struct __kfifo *__kfifo = &__tmp->kfifo; \
    __ret = !kfifo_is_empty(__tmp); \
    if (__ret) { \
        *(typeof(__tmp->type))__val = \
            (__is_kfifo_ptr(__tmp) ? \
            ((typeof(__tmp->type))__kfifo->data) : \
            (__tmp->buf) \
            )[__kfifo->out & __tmp->kfifo.mask]; \
        __kfifo->out++; \
    } \
	__ret; \
}) \
)

#define	kfifo_peek(fifo, val) (\
({ \
	typeof((fifo) + 1) __tmp = (fifo); \
	typeof(__tmp->ptr) __val = (val); \
	uint32_t __ret; \
	struct __kfifo *__kfifo = &__tmp->kfifo; \
    __ret = !kfifo_is_empty(__tmp); \
    if (__ret) { \
        *(typeof(__tmp->type))__val = \
            (__is_kfifo_ptr(__tmp) ? \
            ((typeof(__tmp->type))__kfifo->data) : \
            (__tmp->buf) \
            )[__kfifo->out & __tmp->kfifo.mask]; \
    } \
	__ret; \
}) \
)

#ifdef __cplusplus
}
#endif

#endif /* __KFIFO_H */  