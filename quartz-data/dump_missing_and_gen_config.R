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
#attach(d)
#d1<-d[order(node),][1:2418,] #Pick first 2148 nodes in order
#detach(d)

dNew <- data.frame (NodeID = seq(0,(39*62)-1,by=1))
#dNew$AvgRuntime <- d$avgReRuntime[1:2418]   #Pick 2148 nodes in order
dNew$AvgRuntime <- sample(d1$avgReRuntime, 2418) #Pick 2418 nodes at random
dNew$normRuntime <- (dNew$AvgRuntime-min(dNew$AvgRuntime)) /(max(dNew$AvgRuntime)-min(dNew$AvgRuntime))

#dNew$normRuntime <- d1$runtime_x/median(d1$runtime_x)

dNew$perfClass <- 0
# Top: 10% Class1, 20% Class2, 20% Class3, 25% each Class 4&5
dNew$perfClass[which((dNew$normRuntime <= 0.10))] = 1
dNew$perfClass[which(dNew$normRuntime > 0.10 & dNew$normRuntime <= 0.25)] = 2
dNew$perfClass[which(dNew$normRuntime > 0.25 & dNew$normRuntime <= 0.40)] = 3
dNew$perfClass[which(dNew$normRuntime > 0.40 & dNew$normRuntime <= 0.60)] = 4
dNew$perfClass[which(dNew$normRuntime > 0.60 & dNew$normRuntime <= 1.00)] = 5

# perfpart = floor((39 * 62)/5)         
# d$perfClass[1:perfpart] = 1
# d$perfClass[(perfpart+1):(2*perfpart)] = 2
# d$perfClass[(2*perfpart+1):(3*perfpart)] = 3
# d$perfClass[(3*perfpart+1):(4*perfpart)] = 4
# d$perfClass[(4*perfpart+1):nrow(d)] = 5
#dNew$newNodeID <- d$newNodeID[1:(39 * 62)]
#dNew$perfClass <- d$perfClass[1:(39 * 62)]


#write.table(subset(dNew, select=c(NodeID, perfClass)), file="quartz-dist.csv", row.names = FALSE, col.names = FALSE, quote = FALSE, sep=',')
pdf('dist_graph.pdf', height = 4, width = 4)
par(mar=c(4,4,3,1))
p<-table(dNew$perfClass)
barplot(p, ylim=c(0,1250), width = rep(0.2,5), space = rep (0.5,5), xlim = c(0,1.5), xlab = 'Performance Class', ylab = 'Frequency', las=1)
grid(NA,NULL)
#Add the barplot again so grid lines are in background...argh.
barplot(p, ylim=c(0,1250), width = rep(0.2,5), space = rep (0.5,5), xlim = c(0,1.5), xlab = 'Performance Class', ylab = 'Frequency', main='Quartz Cluster Variation', las=1, add=T)
box()
dev.off()
