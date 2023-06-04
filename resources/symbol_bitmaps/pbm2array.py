#!/usr/bin/env python2
from pprint import pprint
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("pbm_file", type=argparse.FileType('r'), help="PBM Ascii file")
args = parser.parse_args()
pbmfile = args.pbm_file.name
#print(pbmfile.name)
width = 0
height = 1
bit_width = 8
with open(pbmfile) as f:
    read_data = f.readlines()
#pprint(read_data)
dnl = lambda x : x.replace('\n', '')
read_data = (list(map(dnl,  read_data)))
size = (list(map(int,  read_data[2].split(' ') )))
bstring = ''.join(read_data[3:])
blist = list(map(int, bstring))
twoarr = []
print(size)
it = iter(blist)
for y in range(size[height]):
    row = []
    for x in range(size[width]):
        row.append(next(it))
    twoarr.append(row)

pprint(twoarr)
vals = []
for i in range (size[height]//bit_width):
    st_i = i*bit_width
    end_i = ((i+1)*bit_width)
    block = (twoarr[st_i:end_i])
    pprint(block)
    for byte in (list(map(list, zip(*block)))):
        result = 0
        for bit in reversed(byte):
            result = ((result << 1) | bit)
        vals.append(result)

#print(vals)
print("uint8_t %s[%s] = {%s};" % (pbmfile.split('.')[0], len(vals), (', '.join(list(map(str,vals))))))
