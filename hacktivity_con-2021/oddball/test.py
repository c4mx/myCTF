#!/usr/bin/env python3

with open('oddball2', 'r') as f:
    data = [ l.strip() for l in f.readlines() ]

oct_list = []
for i in data:
    if i != '':
        oct_list += i.split(' ')
print(oct_list)


hex_str = ''
for o in oct_list:
    s = format(int(o, 8), 'x')
    if len(s) == 3:
        s = '0' + s

    ss = s[2:] + s[0:2]
    print(ss)
    hex_str += ss


print(hex_str)

f = bytes.fromhex(hex_str)
print(f)


#n = 3
#l = [ data[i:i+n] for i in range(0, len(data), n) ]


#with open('oddball_b', 'wb') as f:
#    f.write(b)



