package edsm

import (
	"bytes"
	"fmt"
	"mime/multipart"
	"net/http"
	"time"

	"elda-go/def"
)

var buf string

// any name
type handler struct {
	// mandatory fields
	name string
	typ  int

	// optional
	url  string
	mpw  *multipart.Writer
	flds map[string]string
}

// register us
func Register() *handler {
	return &handler{
		name: "edsm",
		typ:  def.HANDLER_TYPE_ACTION | def.HANDLER_TYPE_SOURCE,
	}
}

func (self *handler) Init(vars map[string]string) error {
	var err error

	self.flds = make(map[string]string)

	if self.url, err = def.GetStrVar(vars, "url"); err != nil {
		return err
	}

	if self.flds["apiKey"], err = def.GetStrVar(vars, "api_key"); err != nil {
		return err
	}

	if self.flds["commanderName"], err = def.GetStrVar(vars, "commander"); err != nil {
		return err
	}

	if self.flds["fromSoftware"], err = def.GetStrVar(vars, "software_name"); err != nil {
		return err
	}

	if self.flds["fromSoftwareVersion"], err = def.GetStrVar(vars, "software_ver"); err != nil {
		return err
	}

	return nil
}

func (self *handler) Name() string {
	return self.name
}

func (self *handler) Type() int {
	return self.typ
}

// for POST requests
// https://www.edsm.net/en/api-journal-v1
func (self *handler) Push(s string) error {

	body := new(bytes.Buffer)
	mpw := multipart.NewWriter(body)
	defer mpw.Close()

	for key, val := range self.flds {
		mpw.WriteField(key, val)
	}
	mpw.WriteField("message", s)

	req, err := http.NewRequest(http.MethodPost, self.url, bytes.NewReader(body.Bytes()))
	if err != nil {
		return err
	}
	req.Header.Set("Content-Type", mpw.FormDataContentType())

	client := &http.Client{
		Timeout: time.Second * 5,
	}

	rsp, _ := client.Do(req)
	defer rsp.Body.Close()

	if rsp.StatusCode != http.StatusOK {
		fmt.Errorf("POST to '%s' failed: %s", self.url, http.StatusText(rsp.StatusCode))
	}

	return nil
}

// for GET reuqests?
func (self *handler) Pull() (string, error) {
	time.Sleep(time.Second * 3)
	return buf, nil
}

func (self *handler) Done() {
}
