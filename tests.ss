#TESTS
const cnt_var 12.5

char text0[]="sample text"
byte byte_arr1[10]
byte byte_arr2[]={1,2,3}
var var_arr1[5]
var var_arr2[]={1.2,-2.5,3.3}

	
var single_var1
var single_var2=-2.3

func func1() {

	puts("Test:")
}	

func func2(vnum,val1,val2) {
	func1()
	print_ln(vnum,", ", val1," vs ",val2)
#	stop
}

print_ln("\nTest:cnt_var ",cnt_var)
puts("Test byte_arr2:");
puts(byte_arr2)
puts("\nTest text0:");
puts(text0)
#brkpnt
print_ln("\nTest:byte_arr2 ",byte_arr2)

print_ln("Test:var_arr2 ",var_arr2)
print_ln("Test:single_var2 ",single_var2)
single_var1=((single_var2+4.7)/2 * 5)- 2
#stop

func2(1,single_var1,4)
func2(2,size(var_arr2),3)
func2(3,size(text0),11)

single_var1=var_arr2[1]
print_ln("Test var_arr2[1]:",single_var1);

var_arr1=var_arr2
print_ln("Test var_arr1=var_arr2:",var_arr1);

byte_arr1=var_arr2
print_ln("Test byte_arr1=var_arr2:",byte_arr1);

var_arr2=byte_arr1
print_ln("Test var_arr2=byte_arr1:",var_arr2);




