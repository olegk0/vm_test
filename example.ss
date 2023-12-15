#SNAKE GAME
const snake_max_size 100
const d_up 1
const d_down 2
const d_left 3
const d_right 4

byte snake_x[snake_max_size]
byte snake_y[snake_max_size]
var snake_size
var snake_dir


func print_snake(tt) {
	var t
	var new_x=snake_x[0]
	var new_y=snake_y[0]
	if snake_dir == d_left { #left
		new_x=new_x-1
	}else if snake_dir == d_up { #up
		new_y=new_y-1
	}else if snake_dir == d_down { #down
		new_y=new_y+1
	}else if snake_dir == d_right { #right
		new_x=new_x+1
	}
	#curs(new_x,new_y)
	t=scr_sym(new_x,new_y)
	if t != ' '{
		if t=='@'{
			snake_size=snake_size+1
		}else{
			ret_val(1)
			goto f_end
		}
	}
	putc('Q')
	
	t=snake_size-1
	curs(snake_x[t],snake_y[t])
	putc(' ')
	while(t>0){
		snake_x[t]=snake_x[t-1]
		snake_y[t]=snake_y[t-1]
		
		t=t-1
		curs(snake_x[t],snake_y[t])
		putc('o')
	}
	snake_x[0]=new_x
	snake_y[0]=new_y
	ret_val (0)
f_end:
}

	var scr_width = scr_w()
	var scr_height = scr_h()
	
	var key
	var f_skip=0
	
	snake_x[0]=scr_width/2
	snake_y[0]=scr_height/2 + 2
	snake_x[1]=snake_x[0]-1
	snake_y[1]=snake_y[0]
	snake_size=2
	snake_dir=rand(1)+1

	cls()
#brkpnt

	curs(scr_width/2, scr_height/2)
	puts("Use awsd keys, b - break")


	while(1){
		sleep_ms(500-snake_size)
		f_skip=f_skip+1
		if(f_skip>5){
			f_skip=0
			curs(rand(scr_width),rand(scr_height))
			putc('@')
			curs(rand(scr_width),rand(scr_height))
			putc('#')
		}
		key = inkey(0)
		if key == 'b' {
			break
		}else if key == 'a' { #left
			snake_dir=d_left
		}else if key == 'w' { #up
			snake_dir=d_up
		}else if key == 's' { #down
			snake_dir=d_down
		}else if key == 'd' { #right
			snake_dir=d_right
		}
		
		key=print_snake(0)
#print_ln("snake_dir:",snake_dir)
		if key {
			curs(scr_width/2, 5)
			puts("Game Over!!!")
			break
		}
	}



