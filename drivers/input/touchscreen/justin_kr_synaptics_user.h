// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// This file is specifically generated for
//   Date: 09-Jan, 2007

// OTLib 3.2 is customized with the following options:
// * Interrupt Mode of operation (Attention Line)
// * Software I2C on the host processor
// * Button  Features on OneTouch
// * TimeOut For the Attention Line 
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Copyright (c) 2006-2007 Synaptics, Inc.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of 
// this software and associated documentation files (the "Software"), to deal in 
// the Software without restriction, including without limitation the rights to use, 
// copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the 
// Software, and to permit persons to whom the Software is furnished to do so, 
// subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all 
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
// SOFTWARE.
//
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef	_CUSTOMER_OT_H
#define	_CUSTOMER_OT_H

// *******************************************************
// NOTE: User defined: Please modify for your application
// *******************************************************

// change the typedef's to reflect Host processor C-Compiler/Tool Chain
// data types needed for OTLib
typedef unsigned long   OT_U32;
typedef unsigned short  OT_U16;
typedef unsigned char   OT_U8;

typedef signed long   	OT_S32;
typedef signed short  	OT_S16;
typedef signed char   	OT_S8;


// *****************************************************************
// NOTE: User defined parameters: Please modify for your application
// *****************************************************************

// Timeout for OT_U8 OT_Poll_Attn_Line_TimeOut(void);	User implements this function
#define	OT_TIMEOUT 0x500000			// is equal to 50ms 

// *****************************************************************
// NOTE: User defined parameters: Please modify for your application
// *****************************************************************

// Clock streching limit for I2C Software Bitbanging implementation
#define OT_HAL_CLK_STRETCH_LIMIT	5000000		// corresponds to 500ms 

#endif