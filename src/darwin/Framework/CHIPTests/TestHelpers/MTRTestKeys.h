/**
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#import <Foundation/Foundation.h>
#import <Matter/Matter.h>

NS_ASSUME_NONNULL_BEGIN

@interface MTRTestKeys : NSObject <MTRKeypair>

@property (readonly, nonatomic, strong) NSData * ipk;

@property (readonly, nonatomic) NSData * publicKeyData;

// Count of how many times this keypair has been used to signMessageECDSA_DER.
@property (readonly, nonatomic, assign) unsigned signatureCount;

- (instancetype)init;

@end

NS_ASSUME_NONNULL_END
