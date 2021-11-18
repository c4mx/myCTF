from Crypto.Util.number import getPrime
from random import randint
from math import gcd

with open("flag.txt",'r') as f:
	flag = f.read()

p = getPrime(1024)
g = 3
MASK = 2**1024 - 1

def gen_keys():
	x = randint(1, p-2)
	y = pow(g, x, p)
	return (x, y)

def sign(answer: str, x: int):
	while True:
		m = int(asnwer, 16) & MASK
		k = randint(2, p-2)
		if gcd(k, p - 1) != 1:
			continue 
		r = pow(g, k, p)
		s = (m - x*r) * pow(k,-1,p-1) % (p - 1)
		if s == 0:
			continue
		return (r,s)

def verify(answer: str, r: int, s: int, y: int):
	m = int(answer, 16) & MASK
	if any([x <= 0 or x >= p-1 for x in [m,r,s]]):
		return False
	return pow(g, m, p) == (pow(y, r, p) * pow(r, s, p)) % p

def forgey(p, g, y):
	e = 2
	r = g**e * y % p
	s = -r % (p-1)
	m = e*s % (p-1)
	print(f"m length: {len(format(m, 'b'))}")
	return (m,r,s)
	
def exploit(p, g, y):
	m, r, s = forgey(p, g, y)
	print(f"Forgey (m,r,s): {m} {r} {s}")

	my_answer_d = (289531978607 << 1024) + m 
	my_answer = hex(my_answer_d)[2::]
	print(f"my answer:\n{my_answer}")
	print(f"r:\n{r}")
	print(f"s:\n{s}")


def main2():
	x, y = gen_keys()
	print(f"Server's public key (p,g,y): {p} {g} {y}")

	m, r, s = forgey(p, g, y)
	print(f"Forgey (m,r,s): {m} {r} {s}")

	my_answer_d = (289531978607 << 1024) + m 
	my_answer = hex(my_answer_d)[2::]
	print(f"my answer:\n{my_answer}")
	print(f"r:\n{r}")
	print(f"s:\n{s}")
	if verify(my_answer, r, s, y):
		print('Signature valid !')
		answer_bytes = bytes.fromhex(my_answer)
		if b'Cisco' in answer_bytes:
			print("I see you are a fan of Flash!")
	else:
		print('Failed')

def main():
	x, y = gen_keys()
	print(f"Server's public key (p,g,y): {p} {g} {y}")
	m, r, s = forgey(y)
	print(f"Forgey (m,r,s): {m} {r} {s}")
	my_answer = hex(m)[2::] + '626f7468'
	print(f"my answer:\n{my_answer}")
	print(f"r:\n{r}")
	print(f"s:\n{s}")
	print("Who do you think is the tech wizard: Felicity or Cisco or both? Please answer it with your signnature (r,s)")
	print('Answer: ')
	answer = input()
	print('r: ')
	r = int(input())
	print('s: ')
	s = int(input())
	answer_bytes = bytes.fromhex(answer)

	if b'Felicity' not in answer_bytes and b'Cisco' not in answer_bytes and b'both' not in answer_bytes:
		print("Error: the answer is not valid!")
	elif verify(answer, r, s, y):
		print(answer_bytes)
		if b'Felicity' in answer_bytes:
			print("I see you are a fan of Arrow!")
		elif b'Cisco' in answer_bytes:
			print("I see you are a fan of Flash!")
		else:
			print("Brown noser!")
		print(flag)
	else:
		print("Error: message does not match given signature")

if __name__ == "__main__":
	main2()
