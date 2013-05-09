from collections import defaultdict
import sys,re


def unique_everseen(iterable, key=None):
    "List unique elements, preserving order. Remember all elements ever seen."
    from itertools import ifilterfalse
    # unique_everseen('AAAABBBCCDAABBB') --> A B C D
    # unique_everseen('ABBCcAD', str.lower) --> A B C D
    seen = set()
    seen_add = seen.add
    if key is None:
        for element in ifilterfalse(seen.__contains__, iterable):
            seen_add(element)
            yield element
    else:
        for element in iterable:
            k = key(element)
            if k not in seen:
                seen_add(k)
                yield element


extras = {
"TURKISH_REPUBLIC_OF_NORTH_CYPRUS": "X-TURCYP",
"BRITISH_INDIAN_OCEAN_TERRITORY": "X-BO",
"CHRISTMAS_ISLAND": "X-XMAS",
"COCOS_ISLANDS": "X-CC",
}

## ISO code => ISO code mappings
## delete the left side and replace with right side.
merges = {
"MNE": "SRB",
}

# (code, name) pairs to exclude
blacklist = {
("GBR", "IRISH"),  ## also appears under IRL, IRL is better
}


data = open("dict/CountryInfo.111216.txt").read()
data = re.sub(r'\<!--.*?--\>', "", data, flags=re.DOTALL)

code2names = defaultdict(list)
name2codes = defaultdict(list)
code2codeinfo = {}

# print data[:100]
for match in re.finditer(r'\<Country\>.*?\</Country\>', data, re.DOTALL):
    blob = match.group()
    # print blob[:1000]
    m = re.search(r'\<CountryCode\>([A-Z]+)\</', blob)
    code = m.group(1) if m else None
    cow_alpha = re.search(r'\<COW-Alpha\>(.*?)\</COW-Alpha\>', blob).group(1)

    allnames = []

    m = re.search(r'\<CountryName\>(.*?)\</', blob, re.DOTALL)
    allnames.append(m.group(1).strip())

    if code is None:
        code = extras[allnames[0]]

    if code in merges:
        code = merges[code]

    code2codeinfo[code] = {'cow_alpha': cow_alpha}

    m = re.search(r'\<Nationality\>(.*?)\</Nationality', blob, re.DOTALL)
    if m:
        allnames += [x.strip() for x in m.group(1).strip().split('\n')]

    allnames = [x.strip().replace('_',' ') for x in allnames]
    allnames = [re.sub(r'\s+',' ', x) for x in allnames]
    allnames = [x.strip() for x in allnames]
    allnames = [x.replace(' ','_') for x in allnames]

    allnames = [x for x in allnames if (code,x) not in blacklist]
    
    for name in allnames:
        code2names[code].append(name)
        name2codes[name].append(code)

for name,codes in name2codes.items():
    if len(set(codes)) > 1:   ## no ambiguity
        print>>sys.stderr,"AMBIGUITY ||| ", name, " ||| ",codes

for code,names in sorted(code2names.items()):
    names = list(unique_everseen(names))
    print '=== countrycode=%s cowalpha=%s' % (code, cow_alpha)
    # print names
    for name in names:
        print '\t'.join([code, name])
