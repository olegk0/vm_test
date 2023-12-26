#TESTS
const cnt_var 12.5
const cnt_text "Test:"
const cnt_arr {9,8,7,6,5,4}

func func1() {

	puts(cnt_text)
}	

func func2(vnum,val1,val2) {
	func1()
	print(vnum,", ", val1," vs ",val2)
#	stop
}

func pow(base,deg){
	if deg > 0 {
		return(base * pow(base,deg-1))
	}else{
		return(1)
	}
}

func pow0(base,deg){
	var ret=1
	while deg > 0 {
		ret= ret *base
		deg=deg-1
	}
	return(ret)
}

char text0[]="sample text"
byte byte_arr1[10]
byte byte_arr2[]={1,2,3}
var var_arr1[5]
var var_arr2[]={1.2,-2.5,3.3}

var single_var1
var single_var2=-2.3

print_ln(cnt_text,"cnt_var ",cnt_var)
print_ln(cnt_text,"cnt_text ",cnt_text)
print_ln(cnt_text,"cnt_arr ",cnt_arr)
#putc('\n')
#brkpnt
print_ln(cnt_text,"text0:",text0);
#stop
print_ln(cnt_text,"byte_arr2:",byte_arr2);
print_ln(cnt_text,"var_arr2 ",var_arr2)
#putc('\n')
#brkpnt
print_ln(cnt_text,"single_var2 ",single_var2)
putc('\n')
single_var1=((single_var2+cnt_var)/2 * 5)- 20
func2(1,single_var1,5.5)
putc(';')
func2(2,size(var_arr2),3)
putc('\n')
func2(3,size(text0),11)
putc(';')
single_var1=var_arr2[1]
print_ln(cnt_text,"var_arr2[1]:",single_var1);

var_arr1=cnt_arr
print_ln(cnt_text,"var_arr1=cnt_arr:",var_arr1);

var_arr1=var_arr2
print_ln(cnt_text,"var_arr1=var_arr2:",var_arr1);

byte_arr1=var_arr2
print_ln(cnt_text,"byte_arr1=var_arr2:",byte_arr1);

byte_arr1[0]=100
print_ln(cnt_text,"byte_arr1[0]=100:",byte_arr1);

var_arr2=byte_arr1
print_ln(cnt_text,"var_arr2=byte_arr1:",var_arr2);

var_arr2[1]=-12345.6
print_ln(cnt_text,"vvar_arr2[1]=-12345.6:",var_arr2);
#brkpnt
print(cnt_text,"by recursion, 2^8=",pow(2,8),";");

single_var1=pow0(2,8)
print_ln(cnt_text,"by loop, 2^8=",single_var1);




