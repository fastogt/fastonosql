#!/usr/bin/env python2

from rejson import Client, Path


def test():
    rj = Client(host='localhost', port=6379)

    # Set the key `obj` to some object
    obj = {
        'answer': 42,
        'arr': [None, True, 3.14],
        'truth': {
            'coord': 'out there'
        }
    }
    rj.jsonset('obj', Path.rootPath(), obj)

    # Get something
    print 'Is there anybody... {}?'.format(
        rj.jsonget('obj', Path('.truth.coord'))
    )

    # Delete something (or perhaps nothing), append something and pop it
    rj.jsondel('obj', Path('.arr[0]'))
    rj.jsonarrappend('obj', Path('.arr'), 'something')
    print '{} popped!'.format(rj.jsonarrpop('obj', Path('.arr')))

    # Update something else
    rj.jsonset('obj', Path('.answer'), 2.17)

    # And use just like the regular redis-py client
    jp = rj.pipeline()
    jp.set('foo', 'bar')
    jp.jsonset('baz', Path.rootPath(), 'qaz')
    jp.execute()


test()
