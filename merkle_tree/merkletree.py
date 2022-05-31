import math
import hashlib
import random
import sys

#create a merkletree
def create_merkletree(message):
    length = len(message) #二叉树叶子节点个数
    depth  = math.ceil(math.log(length,2)) + 1 #二叉树深度
    merkletree = []
    merkletree.append([hashlib.sha256(i.encode()).hexdigest() for i in message])
    for i in range(depth - 1):
        merkletree_part = [] #用于merkletree的层结构
        length_each = math.floor(len(merkletree[i])/2) #上一层节点的个数
        merkletree_part.extend([hashlib.sha256(merkletree[i][j*2].encode() + merkletree[i][j*2+1].encode()).hexdigest() for j in range(length_each)]) 
        if length_each * 2 != len(merkletree[i]): #如果下层节点无法满足两两归一的要求，将多余的节点直接往上提
            merkletree_part.append(merkletree[i][-1])
            # del merkletree[i][-1] 
        merkletree.append(merkletree_part) #添加进总树的存储结构之中   
    return merkletree

def create_message(length): #length 代表要求生成节点的数目
    choosen = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ'
    message = []
    for i in range(length):
        result = ''.join(random.choice(choosen) for j in range(5)) 
        message.append(result)
    return message #存储最终生成原始信息的列表

def verify_node(message,merkletree): #产生审计节点
    message_hash = (hashlib.sha256(message.encode())).hexdigest() #将消息进行hash用于匹配
    try:
        place_message = merkletree[0].index(message_hash) #位置的范围从0到n-1
    except:
        print("no this message in the tree")
    
    depth = len(merkletree) #二叉树深度
    proof = [] #存储需要用于验证的相关节点数据，审核节点
    for i in range(depth - 1): #不需要加入根节点进行验证
        if place_message % 2 == 0: #位置在两个兄弟节点中的左侧
            if place_message != len(merkletree[i]) - 1: #如果本身不位于当前层的最后一落单节点(无兄弟节点)
                proof.append(['0',merkletree[i][place_message + 1]])
        else: #位置在两兄弟节点的右侧
            proof.append([merkletree[i][place_message - 1],'0'])
        place_message = math.floor(place_message / 2)
        
    return proof

def proof(message,proof_node,root):
    message_hash = (hashlib.sha256(message.encode())).hexdigest() #将消息进行hash用于验证
    for i in proof_node:
        if i[0] == '0':    
            message_hash = hashlib.sha256(message_hash.encode() + i[1].encode()).hexdigest()
        else:
            message_hash = hashlib.sha256(i[0].encode() + message_hash.encode()).hexdigest()
    if message_hash == root:
        print("Validation passes, message is in this merkle tree")
    else:
        print("Validation failed, message isn't in this merkle tree")

    
message = create_message(7)
tree = create_merkletree(message)
proof_node = verify_node(message[6],tree)
proof(message[6],proof_node,tree[-1][0])
