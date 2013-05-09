import sys,os,re
input_cmd = r"""grep -Po "'''.*?'''" wiki.txt"""

def stage1():
    for line in os.popen(input_cmd):
        s = line.strip()
        s = s.replace("[[","").replace("]]","")
        for x in s.split('|'):
            yield x
def stage2():
    for x in stage1():
        yield x
        tokens = x.lower().split()
        if 'of' in tokens:
            yield re.sub(r'\bof.*',"", x)

def stage3():
    for x in stage2():
        x = x.replace("'''","")
        x = re.sub(r'\(.*?\)', "", x)
        x = re.sub(r'\s+', " ", x)
        x = x.strip()
        yield x

def stage4():
    for x in stage3():
        if "Conference" in x: continue
        yield x

for x in stage4():
    print x
