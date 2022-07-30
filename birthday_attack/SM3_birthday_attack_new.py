import random
import time
import sys

MAX = 2 ** 32

vi = 0x7380166f4914b2b9172442d7da8a0600a96f30bc163138aae38dee4db0fb0e4e

t = [0x79cc4519, 0x7a879d8a]

rotl = lambda x, n:((x << n) & 0xffffffff) | ((x >> (32 - n)) & 0xffffffff)

def sm3_ff_j(x, y, z, j):
    if 0 <= j and j < 16:
        ret = x ^ y ^ z
    elif 16 <= j and j < 64:
        ret = (x & y) | (x & z) | (y & z)
    return ret

def sm3_gg_j(x, y, z, j):
    if 0 <= j and j < 16:
        ret = x ^ y ^ z
    elif 16 <= j and j < 64:
        ret = (x & y) | ((~ x) & z)
    return ret

def Int2Bin(a, k): # 整数转二进制数同时依据要求进行长度补全
    res = list(bin(a)[2:])
    for i in range(k - len(res)):
        res.insert(0, '0')
    return ''.join(res)

def sm3_p_0(x):
    return x ^ (rotl(x, 9 % 32)) ^ (rotl(x, 17 % 32))

def sm3_p_1(x):
    return x ^ (rotl(x, 15 % 32)) ^ (rotl(x, 23 % 32))

def T(j):
    if j <= 15:
        return t[0]
    else:
        return t[1]


def sm3_cf(v_i, b_i):
    w = []
    w_1 = []
    for i in range(16):
        temp = b_i[i * 32:(i + 1) * 32]
        w.append(int(temp, 2))
    for j in range(16, 68, 1):
        w.append(0)
        w[j] = sm3_p_1(w[j-16] ^ w[j-9] ^ (rotl(w[j-3], 15 % 32))) ^ (rotl(w[j-13], 7 % 32)) ^ w[j-6]
    for j in range(64):
        w_1.append(0)
        w_1[j] = w[j] ^ w[j+4]
    A = [0]*8
    for i in range(8):
        temp = v_i[32 * i:32 * (i + 1)]
        A.append(int(temp, 2))
    for j in range(64):
        SS1 = rotl((rotl(A[0], 12 % 32) + A[4] + rotl(T(j), j % 32)) & 0xffffffff, 7 % 32)
        SS2 = SS1 ^ rotl(A[0], 12 % 32)
        TT1 = (sm3_ff_j(A[0], A[1], A[2], j) + A[3] + SS2 + w_1[j]) % MAX
        TT2 = (sm3_gg_j(A[4], A[5], A[6], j) + A[7] + SS1 + w[j]) % MAX
        A[3] = A[2]
        A[2] = rotl(A[1], 9 % 32)
        A[1] = A[0]
        A[0] = TT1
        A[7] = A[6]
        A[6] = rotl(A[5], 19 % 32)
        A[5] = A[4]
        A[4] = sm3_p_0(TT2)
        
    temp = Int2Bin(A[0], 32) + Int2Bin(A[1], 32) + Int2Bin(A[2], 32) + \
           Int2Bin(A[3], 32) + Int2Bin(A[4], 32) + Int2Bin(A[5], 32) + \
           Int2Bin(A[6], 32) + Int2Bin(A[7], 32)
    temp = int(temp, 2)
    return temp ^ int(v_i, 2)

def filled(message): #按照SM3要求补全信息符合规范
    message = bin(message)[2:] #除去开头的标识符
    for i in range(4): #防止信息的二进制由于转换缺少位数
        if (len(message) % 4 == 0):
            break
        else:
            message = '0' + message
    length = len(message)
    k = 448 - (length + 1) % 512
    if (k < 0):  # 求取满足等式的最小非负整数k
        k += 512
    message = message + '1' + '0' * k + Int2Bin(length, 64)
    return message

def IterFunction(message): # 按照SM3的实现逻辑进行运算
    n = int(len(message) / 512)
    v = []
    v.append(Int2Bin(vi, 256))
    for i in range(n):
        temp = sm3_cf(v[i], message[512 * i:512 * (i + 1)])
        temp = Int2Bin(temp, 256)
        v.append(temp)
    return v[n]

def SM3(msg):
    msg_1=filled(msg)
    hex(int(msg_1,2))
    res=IterFunction(msg_1)
    result=hex(int(res,2))
    return result[2:]

def Birthday_Attck(n): # 找出两个前nbit相同的hash值
    for i in range(2**(int(n))):
        a1 = random.randint(0,2**n)
        a2 = random.randint(0,2**n)
        if(SM3(a1)[:int(n/4)] == SM3(a2)[:int(n/4)]):
            print('Succeed')
            print("first hash is:\n",SM3(a1))
            print("second hash is:\n",SM3(a2))
            return;
    print('Failed')

print("#--------------------------------------------------------------------#")
print("now, you should input the length of bit you want to match. Require! it must be multiple of 8")
length = int(input())
print("#--------------------------------------------------------------------#\n")
time_start = time.time()
Birthday_Attck(length)
time_end = time.time()
time_c = time_end - time_start
print('the time we use：', time_c, 's')


