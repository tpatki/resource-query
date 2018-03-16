d<-read.csv("rank_runtime.csv")
#hist(d$avgReRuntime)

#Can't do this with outer as vector lengths differ...
nodeArr <- seq(1, 2688, by=1)
sortedList <- sort(as.integer((d$node)))

ans = vapply (nodeArr, function(nodeArr) nodeArr == sortedList, logical(length(sortedList)))
res = colSums(ans)
print(which (res == 0))