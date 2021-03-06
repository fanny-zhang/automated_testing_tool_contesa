/******************************************************************************
 * $Id: NSMenuAdditions.m 6980 2008-10-29 02:33:20Z livings124 $
 *
 * Copyright (c) 2007-2008 Transmission authors and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *****************************************************************************/

#import "NSMenuAdditions.h"

@implementation NSMenu (NSMenuAdditions)

- (void) appendItemsFromMenu: (NSMenu *) menu atIndexes: (NSIndexSet *) indexes atBottom: (BOOL) bottom
{
    NSInteger bottomIndex = bottom ? [self numberOfItems] : 0;
    
    NSMenuItem * item;
    for (NSUInteger i = [indexes lastIndex]; i != NSNotFound; i = [indexes indexLessThanIndex: i])
    {
        item = [[menu itemAtIndex: i] retain];
        [menu removeItemAtIndex: i];
        [self insertItem: item atIndex: bottomIndex];
        [item release];
    }
}

@end
