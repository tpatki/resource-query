d<-read.csv("rank_runtime.csv")
#hist(d$avgReRuntime)

#Can't do this with outer as vector lengths differ...
nodeArr <- seq(1, 2688, by=1)
sortedList <- sort(as.integer((d$node)))
ans = vapply (nodeArr, function(nodeArr) nodeArr == sortedList, logical(length(sortedList)))
res = colSums(ans)
missing = (which (res == 0))
print (missing)

# 5 classes for 39 racks with 62 nodes each. Hardcoded as we are discarding some data.
# Then create performance classes based on the distribution. 

dNew <- data.frame (NodeID = seq (1, (39*62), by=1))
dNew$AvgRuntime <- d$avgReRuntime[1:(39*62)] #Pick 2418 nodes
dNew$normRuntime <- (dNew$AvgRuntime-min(dNew$AvgRuntime)) /(max(dNew$AvgRuntime)-min(dNew$AvgRuntime))

dNew$perfClass <- 0
dNew$perfClass[which((dNew$normRuntime <= 0.20))] = 1
dNew$perfClass[which(dNew$normRuntime > 0.20 & dNew$normRuntime <= 0.40)] = 2
dNew$perfClass[which(dNew$normRuntime > 0.40 & dNew$normRuntime <= 0.60)] = 3
dNew$perfClass[which(dNew$normRuntime > 0.60 & dNew$normRuntime <= 0.80)] = 4
dNew$perfClass[which(dNew$normRuntime > 0.80 & dNew$normRuntime <= 1.00)] = 5

# perfpart = floor((39 * 62)/5)         
# d$perfClass[1:perfpart] = 1
# d$perfClass[(perfpart+1):(2*perfpart)] = 2
# d$perfClass[(2*perfpart+1):(3*perfpart)] = 3
# d$perfClass[(3*perfpart+1):(4*perfpart)] = 4
# d$perfClass[(4*perfpart+1):nrow(d)] = 5
#dNew$newNodeID <- d$newNodeID[1:(39 * 62)]
#dNew$perfClass <- d$perfClass[1:(39 * 62)]


write.table(subset(dNew, select=c(NodeID, perfClass)), file="quartz-dist.csv", row.names = FALSE, col.names = FALSE, quote = FALSE, sep=',')

