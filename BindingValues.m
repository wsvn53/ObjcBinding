#import "BindingValues.h"


@implementation BindingContext
-(void)dealloc {
    NSLog(@"%@ dealloc", self);
}
@end

@implementation BindingCenter

+(instancetype)defaultCenter {
    static BindingCenter *defaultCenter = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        defaultCenter = [[BindingCenter alloc] init];
    });
    return defaultCenter;
}

+(NSString *)keyPathFrom:(NSString *)bindingPath {
    return [bindingPath componentsSeparatedByString:@"."].lastObject;
}

-(void)observeValueForKeyPath:(NSString *)keyPath
                     ofObject:(id)object
                       change:(NSDictionary<NSKeyValueChangeKey,id> *)change
                      context:(void *)context {
    BindingContext *bindingContext = (__bridge BindingContext *)context;
    // avoid loop value set
    if (bindingContext == nil || bindingContext.setLocked) {
        return;
    }
    
    bindingContext.setLocked = YES;
    
#define ExecuteSetBlock(type, block, value)   { \
    typedef void (*SetBlock)(type);             \
    SetBlock setBlockFunc = (SetBlock)imp_implementationWithBlock(block);    \
    setBlockFunc(value);    \
}
    
    // check value type
    id setBlock = (object == bindingContext.srcObject) ? bindingContext.dstSet : bindingContext.srcSet;
    if ([bindingContext.valueType isEqualToString:@"BOOL"]) {
        BOOL value = [change[NSKeyValueChangeNewKey] boolValue];
        ExecuteSetBlock(BOOL, setBlock, value);
    } else if ([@[ @"int", @"NSInteger", @"NSUInteger" ] containsObject:bindingContext.valueType]) {
        NSInteger value = [change[NSKeyValueChangeNewKey] integerValue];
        ExecuteSetBlock(NSInteger, setBlock, value);
    } else {
        ExecuteSetBlock(id, setBlock, change[NSKeyValueChangeNewKey]);
    }
    
    bindingContext.setLocked = NO;
}

@end
