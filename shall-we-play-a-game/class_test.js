Java.perform(function() {
    var MainActivity = Java.use('com.google.ctf.shallweplayagame.GameActivity');
    MainActivity.onCreate.implementation = function(a) {
        this.onCreate(a);
        send({"o: ":this.o});
    };
});
