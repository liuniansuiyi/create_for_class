# -*- coding: utf-8 -*-
"""
Created on Mon Jul  4 14:46:30 2022
说明：随机生成一条需要加密的信息(采用最简单的随机数生成，直接对生成的随机数加密)
同时，根据结果的hash值推出第一次压缩之后各个寄存器里的值
在message+padding之后附加一段消息，用推得的寄存器中的值作为IV去加密附加的消息得到hash_1
加密message+padding+m,得到hash_2
hash_1 == hash_2 => 长度拓展攻击成功
"""
from gmssl import sm3, func
import random
import sm3_change
import struct

# 直接生成一个随机数串作为信息进行SM3加密,并存储该信息的相关参数
message = str(random.random()) 
message_hash = sm3.sm3_hash(func.bytes_to_list(bytes(message, encoding='utf-8'))) 
message_len = len(message)
appending = "wcy201900460021"   # 附加消息
padding_string = ""
padding_list = []


def generate_humancraft_hash(message_hash, message_len, appending):
    vectors = []
    message_0 = ""
    # 将message_hash按照每组8个字节进行分组, 求得第一次压缩之后在各个寄存器当中的值
    for r in range(0, len(message_hash), 8):
        vectors.append(int(message_hash[r:r + 8], 16))

    # 伪造消息,根据SM3的实现原理进行填充和伪造
    if message_len > 64:
        for i in range(0, int(message_len / 64) * 64):
            message_0 += 'x' # 用x作为一个占位符，表示此处已存在元素，下同理
    for i in range(0, message_len % 64):
        message_0 += 'x'
    message_0 = func.bytes_to_list(bytes(message_0, encoding='utf-8'))
    message_0 = padding(message_0)
    message_0.extend(func.bytes_to_list(bytes(appending, encoding='utf-8')))
    
    #利用改编过的IV向量和SM3加密算法进行对现有信息hash值的求取
    return sm3_change.sm3_hash(message_0, vectors)


def padding(message):
    mlen = len(message)
    message.append(0x80)
    mlen += 1
    # 因为添加了结尾的标识符，占用了一个字节，因此长度需要加1
    
    tail = mlen % 64
    range_end = 56
    # 按照SM3的逻辑对信息进行扩充至对应字节长度
    if tail > range_end:
        range_end = range_end + 64
    for i in range(tail, range_end):
        message.append(0x00)
    bit_len = (mlen - 1) * 8
    # 前面长度已经算上了占位符所用的1个字节，需要减去
    
    message.extend([int(x) for x in struct.pack('>q', bit_len)]) 
    for j in range(int((mlen - 1) / 64) * 64 + (mlen - 1) % 64, len(message)):
        global padding_list,padding_string # 对全局变量padding,padding_string重新赋值
        padding_list.append(message[j])
        padding_string += str(hex(message[j]))
    return message


humancraft_hash = generate_humancraft_hash(message_hash, message_len, appending)

new_message = func.bytes_to_list(bytes(message, encoding='utf-8'))
new_message.extend(padding_list)
new_message.extend(func.bytes_to_list(bytes(appending, encoding='utf-8')))
new_message_string = message + padding_string + appending
new_hash = sm3.sm3_hash(new_message)

print("create message:",message)
print("hash of the message:", message_hash)
print("appending message we choice:", appending)
print("------------------------ hash_1 -----------------------------------")
print("compute the hash value of the message we created")
print("hash_humancraft:",humancraft_hash)
print("------------------------ hash_2 -----------------------------------")
print("compute the hash of (message+padding+m)")
print("new message:\n",new_message_string)
print("hash:",new_hash)
print("------------------------ compare ----------------------------------")
if new_hash == humancraft_hash:
    print("success")
else:
    print("fail")
