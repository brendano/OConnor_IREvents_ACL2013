#!/bin/bash
here=$(dirname $0)
# tabari_exe=$here/TABARI.0.8.3.OSX6.dir/TABARI.0.8.3b1
tabari_exe=$here/TABARI.0.8.4.dir/TABARI.0.8.4B1

set -eux

textfile=$1
outfile=$2

cat > tmp.project <<-EOF
<actorsfile> 	$here/dict/CountryInfo.120116.BTOHACK2.actors
<verbsfile> 	$here/dict/CAMEO.080612.verbs
<optionsfile>   $here/dict/CAMEO.09b5.TINYHACK.options
<textfile>      $textfile
<eventfile> 	$outfile
<problemfile>   tmp.$outfile.problems
EOF
# <actorsfile> 	$here/dict/Levant.080629.BTOHACK.actors


# yes | $tabari_exe -aq tmp.project
$tabari_exe -aq tmp.project
mv $outfile.1 $outfile
