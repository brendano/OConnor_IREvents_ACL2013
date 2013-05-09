source("functions.R")
mid=load_mids()

## doesnt matter which model, load any model
m=load_model_basics("~/ptab/bl/251742.v7.v.k=10.c=1.out")

text_dyads=unique(m$dyadvocab$string)
mid_dyads=with(mid, unique(c(paste(sidea,sideb), paste(sideb,sidea))))

# > intersect(mid_dyads, text_dyads)
#  [1] "USA SRB" "USA IRQ" "GBR IRQ" "FRA IRQ" "RUS IRQ" "IRQ SAU" "KWT IRQ" "HRV SRB"
#  [9] "HRV BIH" "SRB BIH" "RUS GEO" "IRQ KWT" "THA KHM" "THA MMR" "MMR THA" "PAK IND"
# [17] "VEN COL" "USA HTI" "PRK CHN" "TWN CHN" "USA PRK" "KOR PRK" "PRK USA" "PRK KOR"
# [25] "PRK JPN" "CHN TWN" "JPN TWN" "PHL CHN" "TUR CYP" "CYP TUR" "TUR GRC" "RUS JPN"
# [33] "ALB SRB" "RUS UKR" "CHN RUS" "RUS AFG" "CHN JPN" "USA CHN" "CHN USA" "CHN KOR"
# [41] "IRN IRQ" "UGA SDN" "GRC TUR" "SRB HRV" "RUS POL" "KOR JPN" "JPN KOR" "EGY SDN"
# [49] "RUS SRB" "TUR IRQ" "IRN AFG" "USA RUS" "SYR ISR" "ISR SYR" "ISR LBN" "CAN USA"
# [57] "USA SYR" "TUR IRN" "USA LBY" "CUB USA" "RUS USA" "USA IRN" "USA SDN" "IND PAK"
# [65] "USA AFG" "SDN UGA" "ERI ETH" "USA VEN" "AUS IDN" "JPN PRK" "DEU IRQ" "ITA IRQ"
# [73] "IRQ TUR" "EGY IRQ" "JOR IRQ" "ISR IRQ" "CAN AFG" "GBR AFG" "FRA AFG" "DEU AFG"
# [81] "PAK AFG" "TUR SYR" "GBR RUS" "IRN TUR" "RUS GBR" "RUS FRA" "RUS DEU"

intersection_dyads=intersect(mid_dyads, text_dyads)

cc=unlist(load_matrix(m,5000,"cContext"))
dyad_counts=daply(data.frame(dyad=with(m$contextvocab, paste(s,r)), n=cc), .(dyad), function(x) sum(x$n))

# the_dyads = names(sort(dyad_counts[the_dyads], dec=T)[1:5])
# 
rm(m, cc)
# 
# the_dyads = c(the_dyads,
#               "RUS IRQ", "USA SYR", "USA LBY", "IRQ TUR")

# the_dyads = intersection_dyads

filter = function(countries) {
  blacklist = c("KSV","PSE","CHNTIB")
  !(countries %in% blacklist) & !str_detect(countries, "^IGO")
}
x = matrix(unlist(strsplit(text_dyads," ")),ncol=2,byrow=T)
good_dyads = text_dyads[ filter(x[,1]) & filter(x[,2]) ]

# dput(good_dyads) ==> big list

rm(x)

