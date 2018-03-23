nn<-read.table('numNodes', header=FALSE)

pdf("job_trace.pdf", width=18, height=7)
par(mfrow=c(2,1), mar=c(5,4,3,1))
plot(nn$V1, xlab="Job ID", ylab="Requested node count", pch='x', col='darkgreen', main = 'Quartz Job Trace Distribution', cex=1, las=1, ylim = c(0,1040))
grid()
box()

par(mar=c(5,4,1,1))
barplot(table(nn$V1), ylim=c(0,50), las=1, xlab = 'Requested node count', ylab = 'Frequency', cex.axis = 1)
grid()
box()
dev.off()