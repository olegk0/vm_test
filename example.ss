
const sname "some string"
const aname {1,2,3,4}

const vname 123
#stop
byte var1[ 4+7 ]
var var2

func fun1(var10,var20) {
		print_ln( var10, var20)
	return 0
}

func pwr2(st) {
	var f
	if st > 1 {
		f =  2 * pwr2(st-1)
	}else{
		f=2
	}
	return f
}
	
	
	cls()
	var width = scr_w()
	var height = scr_h()
	print_ln("\nh:",height," w:",width)
	var x=0
	var y=rand(height/2)
	brkpnt
	cls()
	while(x<width){
		var d_x=rand(d_ampl)
		while(x<width && d_x>0){
		#brkpnt
			var t = rand(45)
			if t < 15 {
				y = y - 1
			}else if t < 30 {
			} else {
				y = y + 1
			}
			
			if y >= height {
				y= height - 1
			} else if y < 0 {
				y = 0 
			}
			
			var t_y=0
			while(t_y <= y){
				curs(x,t_y)
				#print_ln("x:",x," y:",t_y)
				putc('#')
				t_y=t_y+1
			}

			#curs(x,y)
			#putc('#')
			x=x+1
			d_x=d_x-1
		}

		#print_ln(">x:",x," y:",y)
		#x=x+1
	}
	puts("Use awsd keys, b - break")

	var key
	x = width /2
	y = height/2
	var x_p =x-1
	var y_p =y
	while(1){
		key = inkey(0)
		if key = 'b' {
			break
		}else if key = 'a' { #left
			x=x-1
			if x<0 {
				x=0
			}
		}else if key = 'w' { #up
			y=y-1
			if y<0 {
				y=0
			}
		}else if key = 's' { #down
			y=y+1
			if y>=height {
				y=height-1
			}
		}else if key = 'd' { #right
			x=x+1
			if x>=width {
				x=width-1
			}
		}
		
		if x_p != x || y_p != y {
			curs(x_p,y_p)
			putc(' ')
			curs(x,y)
			putc('@')
			x_p=x
			y_p=y
		}
		
	}



