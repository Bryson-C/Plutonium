

class Time {
    foreign static now()
}


class Math {
    foreign static sin(x)
    foreign static add(x,y)
}

var current = 0
while (Time.now() < 1000) {
    if (Math.sin(Time.now()) != current) {
        current = Math.sin(Time.now())
        System.print(current)
    }
}

