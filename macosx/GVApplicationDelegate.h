/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/org/documents/epl-2.0/EPL-2.0.html
 *
 * Contributors: Details at http://www.graphviz.org/
 *************************************************************************/

#import <Cocoa/Cocoa.h>

@class GVAttributeInspectorController;

@interface GVApplicationDelegate : NSObject {
	GVAttributeInspectorController *_attributeInspectorController;
	BOOL _applicationStarted;
}

- (IBAction)showAttributeInspector:(id)sender;

- (BOOL)applicationOpenUntitledFile:(NSApplication*)theApplication;
- (void)applicationDidFinishLaunching:(NSNotification*)aNotification;

@end
