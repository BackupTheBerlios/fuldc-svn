import os
import sys
import re

#make sure relative paths work from the vc7.1 dir too
#os.chdir(sys.argv[0][0: sys.argv[0].rfind("\\")])

ENTRIESSUBPATH = ".svn/entries"
TEMPLATE = "client/versiontemplate.h"
TARGET = "client/version.h"

DEBUG = True

class SVNReport:

	def __init__(self):
		self.revision = 0
		self.data = {}
	
	def walkrevision(self,root):

		for item in os.listdir(root):
			itempath = os.path.join(root, item)
			if os.path.isdir(itempath):
				if item == ".svn":
					self.findrevision(itempath)
				else:
					self.walkrevision(itempath)
			else:
				continue


	def findrevision(self, itempath):
		itempath = os.path.join(itempath,"entries")
		if not os.path.exists(itempath):
			return
		entries = open(itempath, "r").read()
		revisions = re.compile("^\d{1,5}$", re.MULTILINE).findall(entries)
		for rev in revisions:
                    if self.revision < int(rev):
                        self.revision = int(rev)
                        self.data["revision"] = rev


if __name__ == "__main__":
	s = SVNReport()
	s.walkrevision(".")
	versiontemplate = open(TEMPLATE,'r').read()
	print "Getting SVN Information:"
	for key,value in s.data.iteritems():
		versiontemplate = versiontemplate.replace("$%s" % key, value)
	print "Revision %s" % (s.revision)
	print "Checking if update of version.h is needed"
	if not os.path.exists(TARGET):
		open(TARGET,'w').write(versiontemplate)
		print "Updates version.h from template file"
	elif open(TARGET,'r').read() != versiontemplate:
		print "Updates version.h from template file"
		open(TARGET,'w').write(versiontemplate)
	else:
		print "No changes required in version.h."
