from atlas import *

def process():
    con1 = Connection()
    print('1')
    con1.connect("localhost")
    print('2')

    con1.create("afph", "testpw")
    print('3')
    account_template = Entity()
    print('4')
    account_template.parents = []
    print('5')
    account_template.id = 'foo'
    print('6')
    account_template.characters = []
    print('7')
    account_template.objtype = 'foo'
    print('8')
    con1.waitfor("info", account_template)
    return
