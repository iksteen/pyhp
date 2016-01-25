pyhp
====

This extension allows you to embed a PHP interpreter into your python
projects.

Installation
------------

How hard the installation will be depends on a number of factors:

- Your OS.
- Wether your distribution of PHP enables the php-embed SAPI mode.

On Ubuntu, you should *at least* install libphp5-embed and php5-dev, but
you will probably need to install some more development packages.

However, you might need to compile your own PHP.. Don't forget to enable
the --with-embed=shared configure flag. You can safely disable all the
other SAPI modules.

Once you have that covered, you can install this python extension using:

.. code::

    python setup.py install

Usage
-----

.. code:: python

    >>> import pyhp
    >>> pyhp.evaluate('echo $foo;', {'foo': 'bar'})
    'bar'
    >>> pyhp.execute('foo.php', {'foo': 'bar'})

Pretty much any object can be passed from python to PHP. None, strings,
integers, booleans, tuples, lists and dicts are first translated to
their respective PHP type. Objects and callables are passed by a proxy
wrapper which allows access to properties and function / method
invocation.

When calling a python function from PHP code you can pass NULL, integers,
doubles, booleans, strings and arrays and they will be translated to their
respective python types. A heuristic is applied to PHP arrays to determine
if the keys are sequential integers. If so, a list will be passed to the
python callable. Otherwise a dict is passed. Passing PHP objects from PHP
to a python callable is not supported. You can however pass proxied
python objects back to python code.

What people say
---------------

    tech2: excuse me while I get a hacksaw for my skull and a bottle of isopropanol.

    Charlie_X: Personally, I'm quite happy to burn you at the stake for that little demon!

    anonymous: Hurrah! An extra warm spot in hell for you. You've earned it.

    bertjwregeer: Someone for the love of everything please explain why https://github.com/iksteen/pyhp exists. Embedding PHP in Python!!!

License
-------

*pyhp* is distributed under the MIT license.
