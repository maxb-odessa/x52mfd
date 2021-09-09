package igau

import (
	"bufio"
	"fmt"
	"os"

	"elda-go/def"
)

// any name
type handler struct {
	// mandatory fields
	name string
	typ  int

	// optional
	out *bufio.Writer
}

// register us
func Register() *handler {
	return &handler{
		name: "igau",
		typ:  def.HANDLER_TYPE_ACTION | def.HANDLER_TYPE_SOURCE,
	}
}

func (self *handler) Init(vars map[string]string) error {
	self.out = bufio.NewWriter(os.Stdout)
	return nil
}

func (self *handler) Name() string {
	return self.name
}

func (self *handler) Type() int {
	return self.typ
}

func (self *handler) Push(s string) error {
	self.out.WriteString(s)
	self.out.Flush()
	return nil
}

func (self *handler) Pull() (string, error) {
	return "", fmt.Errorf("not implemented")
}

func (self *handler) Done() {
}
