#import <Foundation/Foundation.h>
#import <objc/runtime.h>

@interface BindingCenter : NSObject
+(instancetype)defaultCenter;
+(NSString *)keyPathFrom:(NSString *)bindingPath;

@end

@interface BindingContext : NSObject
@property (nonatomic, assign)   BOOL setLocked;

@property (nonatomic, copy)     NSString *valueType;

@property (nonatomic, weak)     id  srcObject;
@property (nonatomic, weak)     id  dstObject;

@property (nonatomic, copy)     id  srcSet;
@property (nonatomic, copy)     id  dstSet;

@end

static inline void BindingObjectBlocks(const char *valueType,
                                       id srcObject, const char *srcBindingPath,
                                       id dstObject, const char *dstBindingPath,
                                       id srcSet, id dstSet) {
    NSString *srcKeyPath = [BindingCenter keyPathFrom:[NSString stringWithUTF8String:srcBindingPath]];
    NSString *dstKeyPath = [BindingCenter keyPathFrom:[NSString stringWithUTF8String:dstBindingPath]];
    
    BindingContext *context = [[BindingContext alloc] init];
    context.valueType = [NSString stringWithUTF8String:valueType];
    context.srcObject = srcObject;
    context.dstObject = dstObject;
    context.srcSet = srcSet;
    context.dstSet = dstSet;
    
    if (srcObject) {
        // Only used to AutoRelease context
        objc_setAssociatedObject(srcObject, srcKeyPath.UTF8String,
                                 context, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        [srcObject addObserver:BindingCenter.defaultCenter forKeyPath:srcKeyPath
                       options:(NSKeyValueObservingOptionNew) context:(__bridge void *)context];
    }
    
    if (dstObject) {
        // Only used to AutoRelease context
        objc_setAssociatedObject(dstObject, srcKeyPath.UTF8String,
                                 context, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        [dstObject addObserver:BindingCenter.defaultCenter forKeyPath:dstKeyPath
                       options:(NSKeyValueObservingOptionNew) context:(__bridge void *)context];
    }
}

#define BindingValues(valueType, srcObject, srcExpression, dstObject, dstExpression) { \
    __weak typeof(self) bindingWeakSelf = self;                             \
    BindingObjectBlocks(#valueType, srcObject, #srcExpression, dstObject, #dstExpression,    \
    ^(valueType value) {                                                    \
        __strong typeof(bindingWeakSelf) self = bindingWeakSelf; &self;     \
        srcExpression = value;                                              \
    }, ^(valueType value) {                                                 \
        __strong typeof(bindingWeakSelf) self = bindingWeakSelf; &self;     \
        dstExpression = value;                                              \
    });                                                                     \
}

#define BindingValueBlocks(valueType, srcObject, srcExpression, dstBlock) { \
    __weak typeof(self) bindingWeakSelf = self;                             \
    BindingObjectBlocks(#valueType, srcObject, #srcExpression, nil, "",     \
    ^(valueType value) {                                                    \
        __strong typeof(bindingWeakSelf) self = bindingWeakSelf; &self;     \
        srcExpression = value;                                              \
    }, dstBlock);                                                           \
}
