package filein

import (
	"bufio"
	"fmt"
	"os"
	"strings"

	"elda-go/def"
)

// any name
type handler struct {
	// mandatory fields
	name string
	typ  int

	// optional
	in *bufio.Scanner
}

// register us
func Register() *handler {
	return &handler{
		name: "filein",
		typ:  def.HANDLER_TYPE_SOURCE,
	}
}

func (self *handler) Init(vars map[string]string) error {
	self.in = bufio.NewScanner(os.Stdin)
	return nil
}

func (self *handler) Name() string {
	return self.name
}

func (self *handler) Type() int {
	return self.typ
}

func (self *handler) Pull() (string, error) {
	self.in.Scan()
	s := strings.TrimSpace(self.in.Text())
	if err := self.in.Err(); err != nil {
		return "", err
	}
	return s, nil
}

func (self *handler) Push(s string) error {
	return fmt.Errorf("not implemented")
}
