OConnor_IREvents_ACL2013
========================

Replication software, data, and supplementary materials for the paper: O'Connor, Stewart and Smith, ACL-2013, ["Learning to Extract International Relations from Political Context"](http://brenocon.com/oconnor+stewart+smith.irevents.acl2013.pdf).

Website: http://brenocon.com/irevents and
https://github.com/brendano/OConnor_IREvents_ACL2013

Contact: Brendan O'Connor (http://brenocon.com, http://github.com/brendano)

Files contained here (149 MB total; 35 MB without data/):

- paper.pdf: the paper.  [link](http://brenocon.com/oconnor+stewart+smith.irevents.acl2013.pdf)
- supp.pdf: the supplementary appendix.  [link](http://brenocon.com/irevents/supp.pdf)

- data/
  - datefile.week: the timestep system we used for experiments in the paper
  - v7.pathfil.dthresh=500.pthresh=10: the event tuples files we used for
    experiments.  (But not the qualitative evaluation; contact us if you need
    that one, sorry!)  Tab-separated columns are:
      - document ID
      - tuple ID: a string unique for that event tuple within the document. The
        first number is the sentence number.
      - datestamp of the document
      - source actor (ISO3 country code, usually)
      - receiver actor (ISO3 country code, usually)
      - dependency path, encoded as JSON (from Stanford CCprocessed)
        in such a way that it does not contain spaces.
  - v7.pathcounts, v7.dyadcounts: counts from the full dataset, before pruning
    to create the above.

- code/
  - dict/: country/actor dictionaries.
      - country_igos.txt: the list used in experiments.
          - Three-letter codes are ISO3 country codes.
          - IGO* codes are ones we added (from one of the TABARI lists)
          - X-* codes are other ones we added
  - verbdict/: verb-path analysis. Also analysis of TABARI's verb pattern dictionary.
      - cameo_verbs.tsv: an attempt to extract all of TABARI's verb patterns,
        with as minimal alterations as possible
      - See http://brenocon.com/tabari_cameo_verbs.html for a view of what's in
        TABARI's verb pattern dictionary.
      - match.py: matching between our Stanford-style dependency paths, against
        TABARI verb patterns
      - score.py: lexical scale purity evaluation
  - eval/MIDnew: conflict evaluation code, plus plots
  - post/: other posterior analysis scripts, to process saved Gibbs samples
  - preproc/: preprocessing scripts, used to create the event tuples file
  - src/: statistical model code
  - lib/: support libraries for statistical model
      - myutil.jar: has code for many of the statistical inference routines

The code is a little messy because all sorts of auxiliary code has been incuded
for the sake of completeness.  That said, if "java.sh" invokes Java in a good
way for your system, the model should run with

    ./java.sh MainModel run.prop

or to more easily get started use a the "toy settings" options file,

    ./java.sh MainModel toysettings.prop

This will save Gibbs samples and other information to the directory "out".

If "./java.sh" doesn't work, just invoke "java" yourself with all the .jar
files in lib/, ptab.jar, and the root directory on the classpath.

Scripts in the "post/" directory can postprocess it the out/ directory.
Post-processing example.  "post/viewframes.py" shows the "topics", i.e. most
probable verb paths per frame.  For example, if you ran with
`toysettings.prop`, it should have saved a Gibbs sample at iteration number 10
Then run this and open tmp.html in your browser.  

    python post/viewframes.py out/model.10 > tmp.html

10 iterations is not enough to get good semantic coherence.  The topic-word
report will look OK at 1000 iterations, but if you run for 10,000 iterations,
it will be about as good as it will get.

In the `out/` directory, some of the files are:

  - *.vocab files are the vocabularies, i.e. name-to-number mappings.  The
    first string has number 0, the second string has number 1, etc.
  - (prefix).cPathFrame is the matrix of word-topic counts, from a sample.

The prefixes take the form model.NNN, where NNN is the iteration whose Gibbs
sample was saved.  The script average.py averages together a number of Gibbs
samples to calculate posterior mean and variances.

Model code
==========

MainModel.java runs the model.  It contains data management code, as well as
inference procedures for most variables in the model.

ContextModel.java contains the variables and inference for the contextual prior
models.  See the comments at the top of the file for how it fits together.
The `contextModel` option in the properties file can select between two classes
to use:

  - ScaleSmoother runs the "vanilla" model from the paper. (Make sure numDims=1.
    If numDims>1,this code then runs another model that wasn't in the paper.)
  - FrameSmoother is the Gaussian random walk model.

NLP code
========

PathAn.java does the actor identification and event dependency path extraction,
based on pre-parsed input.

"myprolog" is an interpreter for a tiny fragment of Prolog.  I think it was
used only for development and not actually at runtime?
