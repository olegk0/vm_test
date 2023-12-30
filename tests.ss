#TESTS
const cnt_var 12.5
const cnt_text "Test_"
const cnt_arr {9,8,7,6,5,4}

func func0() {
}

func func1(var tnum) {
	print(cnt_text,tnum,":")
}	

func func2(var tnum,var val1,var val2) {
	func1(tnum)
	print(", ", val1," vs ",val2)
#	stop
}

func pow(var base, var deg){
	if deg > 0 {
		return(base * pow(base,deg-1))
	}else{
		return(1)
	}
}

func pow0(var base, var deg){

	var ret=1
	while deg > 0 {
		ret= ret *base
		deg=deg-1
		
	}
	return(ret)
}

func size0(var pnt[]){
brkpnt
	return(size(pnt))
}

char text0[]="sample text"
byte byte_arr1[10]
byte byte_arr2[]={1,2,3}
var var_arr1[5]
var var_arr2[]={1.2,-2.5,3.3}

var single_var1
var single_var2=-2.3
#brkpnt
func1(1)
print("cnt_var ",cnt_var)
print_ln("; ","cnt_text '",cnt_text,"'")

func1(2)
print_ln("cnt_arr ",cnt_arr)
#putc('\n')

func1(3)
print("text0:'",text0,"', size:", size(text0))
brkpnt
print_ln("; ","pnt as arg:",size0(text0));
#stop

func1(4)
print("byte_arr2:",byte_arr2)
print_ln("; ","var_arr2 ",var_arr2)

func1(5)
print("single_var2 ",single_var2,"; ")

single_var1=((single_var2+cnt_var)/2 * 5)- 20
func2(6,single_var1,5.5)
putc('\n')

func2(7,size(var_arr2),3)
putc(';')
func2(8,size(text0),11)
putc('\n')

single_var1=var_arr2[1]
print_ln(cnt_text,"var_arr2[1]:",single_var1)

var_arr1=cnt_arr
print_ln(cnt_text,"var_arr1=cnt_arr:",var_arr1)

var_arr1=var_arr2
print_ln(cnt_text,"var_arr1=var_arr2:",var_arr1)

byte_arr1=var_arr2
print_ln(cnt_text,"byte_arr1=var_arr2:",byte_arr1)

byte_arr1[0]=100
print_ln(cnt_text,"byte_arr1[0]=100:",byte_arr1)

var_arr2=byte_arr1
print_ln(cnt_text,"var_arr2=byte_arr1:",var_arr2)

var_arr2[1]=-12345.6
print_ln(cnt_text,"vvar_arr2[1]=-12345.6:",var_arr2)
#brkpnt
print(cnt_text,"by recursion, 2^8=",pow(2,8),";")

single_var1=pow0(2,8)
print_ln(cnt_text,"by loop, 2^8=",single_var1)






