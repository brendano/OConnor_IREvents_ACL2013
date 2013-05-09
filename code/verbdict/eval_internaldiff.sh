#!/bin/zsh
set -eux

cat list_for_eval.txt | parallel --eta -j8 'python verbdict/score.py internal_diff {}/mean' > v5.internaldiff.log

cat v5.internaldiff.log | awk -vRS=loading '{print $1,$2}' | grep -v scored | ruby -ne 'x=$_.split; s=x[0]; puts ([s[/k=(\d+)/,1], s[/v5\.([a-z]+)/, 1]] + x).join(" ")' > v5.internaldiff.log.txt

# git commit -m "internaldiff logs" v5.internaldiff*

