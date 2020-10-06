#ifndef __SL_DEF_H
#define __SL_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined ( __GNUC__ )
    #ifndef __must_check
        #define __must_check    __attribute__((warn_unused_result))					 //  
    #endif // __must_check
#endif // __GNUC__

#ifdef __cplusplus
}
#endif

#endif /* __SL_DEF_H */