from atlas import *

def process(connections = None):
    if connections==None:
        print('No connections')
    else:
        """ It should now be possible to write continued tests using these
        existing connections """
        print '%i connections' % len(connections)
    con1 = Connection()
    con1.connect('localhost')

    con1.create("afph", "testpw")
    account_template = Entity()
    account_template.parents = []
    account_template.id = 'foo'
    account_template.characters = []
    account_template.objtype = 'foo'
    con1.waitfor("info", account_template)
    return
