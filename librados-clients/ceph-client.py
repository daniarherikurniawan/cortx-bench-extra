# Python Code to Test Librados

import rados

try:
        cluster = rados.Rados(conffile='')
except TypeError as e:
        print 'Argument validation error: ', e
        raise e

print "Created cluster handle."

try:
        cluster.connect()
except Exception as e:
        print "connection error: ", e
        raise e
finally:
        print "Connected to the cluster."

print "\n\nI/O Context and Object Operations"
print "================================="

print "\nCreating a context for the 'mypool' pool"
if not cluster.pool_exists('mypool'):
        raise RuntimeError('No data pool exists')
ioctx = cluster.open_ioctx('mypool')

print "\nWriting object 'hw' with contents 'Hello World!' to pool 'mypool'."
ioctx.write("hw", "Hello World!")
print "Writing XATTR 'lang' with value 'en_US' to object 'hw'"
ioctx.set_xattr("hw", "lang", "en_US")


print "\nWriting object 'bm' with contents 'Bonjour tout le monde!' to pool 'mypool'."
ioctx.write("bm", "Bonjour tout le monde!")
print "Writing XATTR 'lang' with value 'fr_FR' to object 'bm'"
ioctx.set_xattr("bm", "lang", "fr_FR")

print "\nContents of object 'hw'\n------------------------"
print ioctx.read("hw")

print "\n\nGetting XATTR 'lang' from object 'hw'"
print ioctx.get_xattr("hw", "lang")

print "\nContents of object 'bm'\n------------------------"
print ioctx.read("bm")

print "Getting XATTR 'lang' from object 'bm'"
print ioctx.get_xattr("bm", "lang")


print "\nRemoving object 'hw'"
ioctx.remove_object("hw")

print "Removing object 'bm'"
ioctx.remove_object("bm")

print "\nClosing the connection."
ioctx.close()

print "Shutting down the handle."
cluster.shutdown()