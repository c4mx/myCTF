# wrapper.sh
#!/bin/sh

# ./wrapper prog port
socat -dd TCP4-LISTEN:"$2",bind=127.0.0.1,fork,reuseaddr EXEC:"$1",pty,echo=0,raw
