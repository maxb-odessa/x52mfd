package def

import (
	"fmt"
	"os"
	"strconv"
)

const (
	SRC_CHAN_LEN = 32
	ACT_CHAN_LEN = 64
)

const (
	HANDLER_TYPE_SOURCE = 1
	HANDLER_TYPE_ACTION = 2
)

var ConfFile string

type ChanMsg struct {
	Name string
	Data string
}

func init() {
	ConfFile = os.Getenv("HOME") + "/" + ".local/etc/elda-go.conf"
}

func GetStrVar(vars map[string]string, varname string) (string, error) {
	if val, ok := vars[varname]; !ok {
		return "", fmt.Errorf("variable '%s' not set")
	} else {
		return val, nil
	}
}

func GetIntVar(vars map[string]string, varname string) (int, error) {
	if val, ok := vars[varname]; !ok {
		return 0, fmt.Errorf("variable '%s' not set", varname)
	} else if i, err := strconv.ParseInt(val, 10, 0); err != nil {
		return 0, err
	} else {
		return int(i), nil
	}
}
