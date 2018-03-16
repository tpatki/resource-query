d<-read.table('perfClassExample.dat', header = T)

pdf("distEx.pdf", width=4, height=4)
par(mar=c(4,4,3,2))
plot(d$class, ylab="Perf Class", xlab="Node ID")
dev.off()