package def

import "os"

const (
	SRC_CHAN_LEN = 32
	ACT_CHAN_LEN = 64
)

const (
	HANDLER_TYPE_SOURCE = 1
	HANDLER_TYPE_ACTION = 2
)

var ConfFile string

func init() {
	ConfFile = os.Getenv("HOME") + "/" + ".local/etc/elda-go.conf"
}
