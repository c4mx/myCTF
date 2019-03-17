#!/bin/usr/env python
# -*- coding: utf-8 -*-

from string import ascii_letters, digits

x_code = "function x(х){ord=Function.prototype.call.bind(''.charCodeAt);chr=String.fromCharCode;str=String;function h(s){for(i=0;i!=s.length;i++){a=((typeof a=='undefined'?1:a)+ord(str(s[i])))%65521;b=((typeof b=='undefined'?0:b)+a)%65521}return chr(b>>8)+chr(b&0xFF)+chr(a>>8)+chr(a&0xFF)}function c(a,b,c){for(i=0;i!=a.length;i++)c=(c||'')+chr(ord(str(a[i]))^ord(str(b[i%b.length])));return c}for(a=0;a!=1000;a++)debugger;x=h(str(x));source=/Ӈ#7ùª9¨M¤À.áÔ¥6¦¨¹.ÿÓÂ.Ö£JºÓ¹WþÊmãÖÚG¤¢dÈ9&òªћ#³­1᧨/;source.toString=function(){return c(source,x)};try{console.log('debug',source);with(source)return eval('eval(c(source,x))')}catch(e){}}" 
source = "Ӈ#7ùª9¨M¤À.áÔ¥6¦¨¹.ÿÓÂ.Ö£JºÓ¹WþÊmãÖÚG¤¢dÈ9&òªћ#³­1᧨"
mess = "¢×&Ê´cÊ¯¬$¶³´}ÍÈ´T©Ð8Í³Í|Ô÷aÈÐÝ&¨þJ"
charset = ascii_letters + digits + "_@!?-"

# Hash function
def h(s, a, b):
    for i in range(len(s)):
        a = (a + ord(s[i])) % 65521
        b = (b + a) % 65521
    return chr(b >> 8) + chr(b & 0xFF) + chr(a >> 8) + chr(a & 0xFF)

# XOR function
def c(s, key):
    r = ""
    for i in range(len(s)):
        r += chr(ord(str(s[i])) ^ ord(key[i % len(key)]))
    return r

def checkbyte(s, c, pos):
    for i in range(pos, len(s), 4):
        if chr(ord(s[i]) ^ ord(c)) not in charset:
            return False
    return True

def brute_force():
    for pos in range(4):
        for i in range(256):
            if checkbyte(mess, chr(i), pos):
                print(f"pos: {pos}, code: {i} -> OK")

x = h(x_code, 1000, 0)
for i in x:
    print(ord(i))

print(c(source, x))

brute_force()

# Two possible hash
hash1 = chr(253) + chr(149) + chr(21) + chr(249)
hash2 = chr(253) + chr(153) + chr(21) + chr(249)

print(c(mess, hash1))
print(c(mess, hash2))
