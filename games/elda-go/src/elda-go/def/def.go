package def

import "os"

const (
	SRC_CHAN_LEN = 32
	ACT_CHAN_LEN = 64
)

var ConfFile string

type ChanMsg struct {
	name string
	data string
}

func init() {
	ConfFile = os.Getenv("HOME") + "/" + ".local/etc/elda-go.conf"
}
