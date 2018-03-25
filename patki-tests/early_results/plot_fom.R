bl<-read.table('baseline_low_fom.dat', header = TRUE)
bh<-read.table('baseline_high_fom.dat', header = TRUE)
v<-read.table('varaware_fom.dat', header = TRUE)

pdf ("fom_per_job.pdf", width=18, height=8)
par(mar = c(3,4,1,1), mfrow=c(2,1))

plot(bl$JobID, bl$Diff, xlab = "JobID", ylab = "Difference in Perf Classes", las = 1, pch = 4, col = 'red', cex = 0.8, ylim = c (0,6))
grid()
points(bh$JobID,bh$Diff, pch = 2, col = 'gray10', cex = 0.8)
points(v$JobID,v$Diff, pch = 1, col = 'blue', cex = 0.8)
legend("topleft", c("Baseline: Lowest ID First", "Baseline: Highest ID First", "Variation Aware: Most Efficient Node First"), pch = c(4, 2, 1), col = c('red', 'gray10', 'blue'), cex=0.8)
box()

par(mar = c(5,4,0,1))
plot(bl$JobID, bl$MaxPC, xlab = "JobID", ylab = "Worst Perf Class Allocated", las = 1, pch = 4, col = 'red', cex = 0.8, ylim = c (0,7))
grid()
points(bh$JobID,bh$MaxPC, pch = 2, col = 'gray10', cex = 0.8)
points(v$JobID,v$MaxPC, pch = 1, col = 'blue', cex = 0.8)
legend("topleft", c("Baseline: Lowest ID First", "Baseline: Highest ID First", "Variation Aware: Most Efficient Node First"), pch = c(4, 2, 1), col = c('red', 'gray10', 'blue'), cex=0.8)
box()
dev.off()

pdf('fom_hist.pdf', width=10, height=5)
par(mfrow=c(1,3))
barplot(table(bh$Diff), ylim=c(0,200), xlim=c(0,1.5), width = rep(0.2,5), space = rep (0.5,5), xlab="Difference in Perf Class", ylab = "Frequency (Number of Jobs)")
grid(NA,NULL)
legend('topright', "Baseline: Highest ID First")
box()


barplot(table(bl$Diff), ylim=c(0,200), xlim=c(0,1.5),width = rep(0.2,5), space = rep (0.5,5), xlab="Difference in Perf Class", ylab = "Frequency (Number of Jobs)")
grid(NA,NULL)
legend('topright', "Baseline: Lowest ID First")
box()

barplot(table(v$Diff), ylim=c(0,200), xlim=c(0,1.5),width = rep(0.2,4), space = rep (0.5,4), xlab="Difference in Perf Class", ylab = "Frequency (Number of Jobs)")
grid(NA,NULL)
legend('topright', "Variation Aware: \nMost Efficient Node First")
box()
dev.off()




