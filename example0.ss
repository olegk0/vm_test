
const sname "some string"
const aname {1,2,3,4}

const vname 123
#stop
byte var1[ 4+7 ]
var var2

func fun1( prm1,  prm2) {
	if prm1 > prm2 {
		print ("var1:",prm1)
	}else{
		print ("var2:",prm2)
	}	
	return prm1+prm2
}

	print(sname)
	print(vname)
	fun1(vname,5)
brkpnt
	cls()
	curs (1,2)
	rnd(5)

	var1[ 1+5 ] = 99
	var2 = 2
	var1[  3 - var2 ] = 123
	var1[ 0 ] = var1[ 1]
	var dd=1
	var gg= -dd
	gg= -1
	var v = gg - dd+5
	
	V = ((3 + gg) * 10 - dd * 2) / 10

	#rem
	 #rem text

	#
	begin: print ("V: ",V, " gg:",gg, dd )
	#stop
	var f=0
	while f<4 {
		f=f+1
	}
brkpnt
	if gg <0 {
		print( "ddd")
	}

	gg=1
	if gg >0 {
		print( "ddd")
	}else{
		print ("ggg")
	}

up:
	gg=5
	if gg = 0 {
		#print( "0")
	}else if gg = 1 {
		goto end
	}else{
		goto up
	}

	fun1(gg,5)
end:

	f=2.125+1.5
	
	f = rnd(5)
	
#	f=var1
#	stop 

