package poodle

/*
#cgo CFLAGS: -I${SRCDIR}
#cgo LDFLAGS: -L${SRCDIR}/build/Debug/ -L${SRCDIR}/build/Release/ -L${SRCDIR}/ -lpoodle

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

extern void* oodle_new(void*);
extern bool oodle_decode(void* comp, void* input, size_t input_len, void* output, size_t output_len);
extern void oodle_destroy(void* comp);
*/
import "C"
import (
	"errors"
	"unsafe"
)

type Decoder struct {
	ptr unsafe.Pointer
}

var ErrFailedCreate = errors.New("failed to create new oodle decoder")
var ErrFailedDecode = errors.New("failed to decode oodle data")

func NewDecoder(data []byte) (*Decoder, error) {
	decoder := C.oodle_new(unsafe.Pointer(&data[0]))
	if decoder == nil {
		return nil, ErrFailedCreate
	}

	return &Decoder{
		ptr: decoder,
	}, nil
}

func (d *Decoder) Decode(input []byte, outputSize int32) ([]byte, error) {
	safe := make([]byte, outputSize)

	result := C.oodle_decode(d.ptr, unsafe.Pointer(&input[0]), C.size_t(len(input)), unsafe.Pointer(&safe[0]), C.size_t(outputSize))
	if !result {
		return nil, ErrFailedDecode
	}

	return safe, nil
}

func (d *Decoder) Cleanup() {
	C.oodle_destroy(d.ptr)
}
