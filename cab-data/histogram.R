d<-read.table("EPtime115.dat", header=FALSE)
vals <- (d$V2 - min(d$V2))/min(d$V2)

d1<-read.table("MGtime115.dat", header=FALSE)
vals1 <- (d1$V2 - min(d1$V2))/min(d1$V2)

d2<-read.table("EPtime065.dat", header=FALSE)
vals2 <- (d2$V2 - min(d2$V2))/min(d2$V2)

d3<-read.table("MGtime065.dat", header=FALSE)
vals3 <- (d3$V2 - min(d3$V2))/min(d3$V2)

pdf("histVarCab.pdf", width=6, height=5)
par(mfrow=c(2,2), mar=c(4,4,3,2), mgp=c(2.1,0.4,0), las=1)
hist(vals, xlab="Percentage slowdown", ylim=c(0,1000), xlim=c(0,0.10), main = "EP 115W", cex.lab=1.1)
grid()
box()

hist(vals1, xlab="Percentage slowdown", ylim=c(0,1000), xlim=c(0,0.10), main = "MG 115W", cex.lab=1.1)
grid()
box()

hist(vals2, xlab="Percentage slowdown", ylim=c(0,1000), xlim=c(0,0.10), main = "EP 65W", cex.lab=1.1)
grid()
box()

hist(vals3, xlab="Percentage slowdown", ylim=c(0,1000), xlim=c(0,0.10), main = "MG 65W", cex.lab=1.1)
grid()
box()
dev.off()