from atlas import *

def process():
    con1 = Connection()
    con1.connect("localhost")

    con1.create("afph", "testpw")
    account_template = Entity()
    account_template.parents = []
    account_template.id = 'foo'
    account_template.characters = []
    account_template.objtype = 'foo'
    con1.waitfor("info", account_template)
    return
