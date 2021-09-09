package def

import (
	"fmt"
	"os"
	"strconv"
	"strings"
)

const (
	SRC_CHAN_LEN = 128
	ACT_CHAN_LEN = 128
)

const (
	HANDLER_TYPE_SOURCE = 0x01
	HANDLER_TYPE_ACTION = 0x02
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
		return "", fmt.Errorf("variable '%s' not set", varname)
	} else if len(val) == 0 {
		return "", fmt.Errorf("varible '%s' is zero", varname)
	} else {
		return val, nil
	}
}

func GetIntVar(vars map[string]string, varname string) (int, error) {
	if val, err := GetStrVar(vars, varname); err != nil {
		return 0, err
	} else if i, err := strconv.ParseInt(val, 10, 0); err != nil {
		return 0, err
	} else {
		return int(i), nil
	}
}

func IsYes(s string) bool {
	switch strings.ToLower(s) {
	case "off", "no", "false", "0":
		return false
	}
	return true
}

func IsVarSetAndYes(vars map[string]string, varname string) bool {
	if val, err := GetStrVar(vars, varname); err != nil {
		return false
	} else {
		return IsYes(val)
	}
}

func SendMsg(ch chan *ChanMsg, msg *ChanMsg) (ok bool) {
	defer func() {
		if recover() != nil {
			ok = false
		}
	}()

	ch <- msg

	return true
}
