b<-read.table('baseline_sched_time', header = FALSE)
v<-read.table('varaware_sched_time', header = FALSE)

pdf("sched_time.pdf", width = 8, height = 6)
par(mar = c(3,4,1,1), mfrow=c(2,1))

plot(b$V1, xlim=c(1,200), ylim = c(0,1.5), ylab = 'Scheduling Time (s)', las = 1, xlab = "", col = 'red', pch ='x', cex = 0.6)
grid()
abline(v=65, lty = 2, col = 'darkseagreen4', lwd=1)
legend('topleft', "Baseline: Highest ID First\nAllocated IDs: 1-61,64, Reserved IDs: 62-63,65-200", pch = 'x', col = 'red', cex = 0.8)
legend('topright', paste("Total: ",round(sum(b$V1),2), 's', sep=''), cex=0.8)

par(mar = c(4,4,0,1))
plot(v$V1, xlim=c(1,200), ylim = c(0,1.5), ylab = 'Scheduling Time (s)', las = 1, xlab = "Job ID", col = 'blue', pch ='o', cex = 0.6)
grid()
abline(v=65, lty = 2, col = 'darkseagreen4', lwd=1)
legend('topleft', "Variation Aware: Most Efficient Node First\nAllocated IDs: 1-61,64, Reserved IDs: 62-63,65-200", pch = 'o', col = 'blue', cex = 0.8)
legend('topright', paste("Total: ",round(sum(v$V1),2), 's', sep=''), cex=0.8)
dev.off()