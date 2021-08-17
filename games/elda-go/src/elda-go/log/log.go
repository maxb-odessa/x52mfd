package log

import (
	"fmt"
	"os"
	"time"
)

// log error message
func genlog(level string, format string, args ...interface{}) {
	fmt.Fprintf(os.Stderr, time.Now().Format("15:04:05 01-02-2006")+" ["+level+"] "+format, args...)
}

// Err logs ERROR messages to stderr
func Err(format string, args ...interface{}) {
	genlog("ERR", format, args...)
}

// Info logs INFO messages to stderr
func Info(format string, args ...interface{}) {
	genlog("INFO", format, args...)
}

// Warn logs WARNING messages to stderr
func Warn(format string, args ...interface{}) {
	genlog("WARN", format, args...)
}
