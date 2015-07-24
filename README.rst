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

Very little interoperability between python and PHP is provided. The
only types that can be passed from python to PHP are: None, strings,
integers, booleans, floats, tuples, lists and dicts. The output of the
PHP script is captured and returned by the evaluate and execute functions.

License
-------

*pyhp* is distributed under the MIT license.
