package fileout

import (
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
	fp *os.File
}

// register us
func Register() *handler {
	return &handler{
		name: "fileout",
		typ:  def.HANDLER_TYPE_ACTION,
	}
}

func (self *handler) Init(vars map[string]string) error {

	var path string
	var err error

	if path, err = def.GetStrVar(vars, "path"); err != nil {
		return err
	}

	flags := os.O_WRONLY

	if def.IsVarSetAndYes(vars, "truncate") {
		flags |= os.O_TRUNC
	}

	if def.IsVarSetAndYes(vars, "create") {
		flags |= os.O_CREATE
	}

	if self.fp, err = os.OpenFile(path, flags, 0666); err != nil {
		return err
	}

	self.fp.Seek(0, os.SEEK_END)

	return nil
}

func (self *handler) Name() string {
	return self.name
}

func (self *handler) Type() int {
	return self.typ
}

func (self *handler) Push(s string) error {
	_, err := self.fp.WriteString(s)
	self.fp.Sync()
	return err
}

func (self *handler) Pull() (string, error) {
	return "", fmt.Errorf("not implemented")
}

func (self *handler) Done() {
	if self.fp != nil {
		self.fp.Close()
		self.fp = nil
	}
}
