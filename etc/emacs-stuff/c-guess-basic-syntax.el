;;; This function is from emacs, and is of course covered by the same
;;; copyright as that of emacs, and in particular cc-mode.el: The GPL,
;;; held by the FSF and probably Barry Warsaw.  I do not wish to claim
;;; any rights on this code.  --Arnt

;;; a message I sent to the qt-interest mailing list, about the hack:

;;; Sender: agulbra@troll.no
;;; To:	qt-interest@troll.no
;;; Subject: Re: Emacs and Qt classes.
;;; References: <199701232120.PAA20383@babba.cu-online.com>
;;; From: Arnt Gulbrandsen <agulbra@troll.no>
;;; Date: 23 Jan 1997 23:33:11 +0100
;;; In-Reply-To: Otto Hammersmith's message of Thu, 23 Jan 1997 15:20:38 -0600 (CST)
;;; Message-ID: <m3n2u058ew.fsf@lupinella.troll.no>
;;; X-Mailer: Gnus v5.2.25/XEmacs 19.14
;;; Return-Path: <owner-qt-interest@troll.no>
;;; Lines: 34
;;; Xref: lupinella.troll.no qt.interest:168 qt.misc:639
;;; 
;;; Otto Hammersmith <ohammers@cu-online.com>
;;; > I've finally gotten the time to write some Qt code (it's amazing how
;;; > much time you have to program after buying a laptop :) and I'm running
;;; > into some problems editing with Emacs C++ mode.
;;; 
;;; So did I.  *sigh*
;;; 
;;; > Whenever I write a class that includes the 'public slots' section,
;;; > cc-mode will get really confused and start indenting things very
;;; > incorrectly.  I saved a post from Arnt a while about about how he has
;;; > Emacs configured, and something I found in there solves the problem
;;; > for 'signals'... but not 'slots' because the extra space, I presume.
;;; > 
;;; > Has anyone out there come up with a solution, or at least a
;;; > workaround, so that other code formatting isn't messed up?
;;; 
;;; Well.  At the end of that message, didn't I write that I had some
;;; more?  I didn't include the "more" part, because it's not pretty.
;;; 
;;; I hacked c-guess-basic-syntax to do the right thing, and I'll say it
;;; again: It's not pretty.  The problem is that c-guess-basic-syntax
;;; assumes that the access specifier is a sexp (a word, more or less),
;;; and my solution is... read the code.  It's in ftp://ftp.troll.no/tmp/.
;;; Add it to your .emacs and make sure to use it rather than the one in
;;; cc-mode.el.
;;; 
;;; I really don't like to do evil hacks like this one.  But I don't want
;;; to rewrite any 700-line fsf functions, either.  And not to live with
;;; messy indentation.
;;; 
;;; Oh, for a harmonious world of peace, kindness and good code.
;;; 
;;; --Arnt

(defun c-guess-basic-syntax ()
  ;; guess the syntactic description of the current line of C++ code.
  (save-excursion
    (save-restriction
      (beginning-of-line)
      (let* ((indent-point (point))
	     (case-fold-search nil)
	     (fullstate (c-parse-state))
	     (state fullstate)
	     (in-method-intro-p (and (eq major-mode 'objc-mode)
				     (looking-at c-ObjC-method-key)))
	     literal containing-sexp char-before-ip char-after-ip lim
	     syntax placeholder c-in-literal-cache inswitch-p
	     ;; narrow out any enclosing class
	     (inclass-p (c-narrow-out-enclosing-class state indent-point))
	     )

	;; get the buffer position of the most nested opening brace,
	;; if there is one, and it hasn't been narrowed out
	(save-excursion
	  (goto-char indent-point)
	  (skip-chars-forward " \t}")
	  (skip-chars-backward " \t")
	  (while (and state
		      (not in-method-intro-p)
		      (not containing-sexp))
	    (setq containing-sexp (car state)
		  state (cdr state))
	    (if (consp containing-sexp)
		;; if cdr == point, then containing sexp is the brace
		;; that opens the sexp we close
		(if (= (cdr containing-sexp) (point))
		    (setq containing-sexp (car containing-sexp))
		  ;; otherwise, ignore this element
		  (setq containing-sexp nil))
	      ;; ignore the bufpos if its been narrowed out by the
	      ;; containing class
	      (if (<= containing-sexp (point-min))
		  (setq containing-sexp nil)))))

	;; set the limit on the farthest back we need to search
	(setq lim (or containing-sexp
		      (if (consp (car fullstate))
			  (cdr (car fullstate))
			nil)
		      (point-min)))

	;; cache char before and after indent point, and move point to
	;; the most likely position to perform the majority of tests
	(goto-char indent-point)
	(skip-chars-forward " \t")
	(setq char-after-ip (following-char))
	(c-backward-syntactic-ws lim)
	(setq char-before-ip (preceding-char))
	(goto-char indent-point)
	(skip-chars-forward " \t")

	;; are we in a literal?
	(setq literal (c-in-literal lim))

	;; now figure out syntactic qualities of the current line
	(cond
	 ;; CASE 1: in a string.
	 ((memq literal '(string))
	  (c-add-syntax 'string (c-point 'bopl)))
	 ;; CASE 2: in a C or C++ style comment.
	 ((memq literal '(c c++))
	  ;; we need to catch multi-paragraph C comments
	  (while (and (zerop (forward-line -1))
		      (looking-at "^[ \t]*$")))
	  (c-add-syntax literal (c-point 'bol)))
	 ;; CASE 3: in a cpp preprocessor
	 ((eq literal 'pound)
	  (c-beginning-of-macro lim)
	  (c-add-syntax 'cpp-macro (c-point 'boi)))
	 ;; CASE 4: in an objective-c method intro
	 (in-method-intro-p
	  (c-add-syntax 'objc-method-intro (c-point 'boi)))
	 ;; CASE 5: Line is at top level.
	 ((null containing-sexp)
	  (cond
	   ;; CASE 5A: we are looking at a defun, class, or
	   ;; inline-inclass method opening brace
	   ((= char-after-ip ?{)
	    (cond
	     ;; CASE 5A.1: we are looking at a class opening brace
	     ((save-excursion
		(goto-char indent-point)
		(skip-chars-forward " \t{")
		;; TBD: watch out! there could be a bogus
		;; c-state-cache in place when we get here.  we have
		;; to go through much chicanery to ignore the cache.
		;; But of course, there may not be!  BLECH!  BOGUS!
		(let ((decl
		       (if (boundp 'c-state-cache)
			   (let ((old-cache c-state-cache))
			     (prog2
				 (makunbound 'c-state-cache)
				 (c-search-uplist-for-classkey (c-parse-state))
			       (setq c-state-cache old-cache)))
			 (c-search-uplist-for-classkey (c-parse-state))
			 )))
		  (and decl
		       (setq placeholder (aref decl 0)))
		  ))
	      (c-add-syntax 'class-open placeholder))
	     ;; CASE 5A.2: brace list open
	     ((save-excursion
		(c-beginning-of-statement-1 lim)
		;; c-b-o-s could have left us at point-min
		(and (bobp)
		     (c-forward-syntactic-ws indent-point))
		(setq placeholder (point))
		(and (or (looking-at "enum[ \t\n]+")
			 (= char-before-ip ?=))
		     (save-excursion
		       (skip-chars-forward "^;" indent-point)
		       (/= (following-char) ?\;))))
	      (c-add-syntax 'brace-list-open placeholder))
	     ;; CASE 5A.3: inline defun open
	     (inclass-p
	      (c-add-syntax 'inline-open (aref inclass-p 0)))
	     ;; CASE 5A.4: ordinary defun open
	     (t
	      (goto-char placeholder)
	      (c-add-syntax 'defun-open (c-point 'bol))
	      )))
	   ;; CASE 5B: first K&R arg decl or member init
	   ((c-just-after-func-arglist-p)
	    (cond
	     ;; CASE 5B.1: a member init
	     ((or (= char-before-ip ?:)
		  (= char-after-ip ?:))
	      ;; this line should be indented relative to the beginning
	      ;; of indentation for the topmost-intro line that contains
	      ;; the prototype's open paren
	      ;; TBD: is the following redundant?
	      (if (= char-before-ip ?:)
		  (forward-char -1))
	      (c-backward-syntactic-ws lim)
	      ;; TBD: is the preceding redundant?
	      (if (= (preceding-char) ?:)
		  (progn (forward-char -1)
			 (c-backward-syntactic-ws lim)))
	      (if (= (preceding-char) ?\))
		  (backward-sexp 1))
	      (c-add-syntax 'member-init-intro (c-point 'boi))
	      ;; we don't need to add any class offset since this
	      ;; should be relative to the ctor's indentation
	      )
	     ;; CASE 5B.2: K&R arg decl intro
	     (c-recognize-knr-p
	      (c-add-syntax 'knr-argdecl-intro (c-point 'boi))
	      (and inclass-p (c-add-syntax 'inclass (aref inclass-p 0))))
	     ;; CASE 5B.3: nether region after a C++ func decl
	     (t
	      (c-add-syntax 'ansi-funcdecl-cont (c-point 'boi))
	      (and inclass-p (c-add-syntax 'inclass (aref inclass-p 0))))
	     ))
	   ;; CASE 5C: inheritance line. could be first inheritance
	   ;; line, or continuation of a multiple inheritance
	   ((or (and c-baseclass-key (looking-at c-baseclass-key))
		(and (or (= char-before-ip ?:)
			 (= char-after-ip ?:))
		     (save-excursion
		       (c-backward-syntactic-ws lim)
		       (if (= char-before-ip ?:)
			   (progn
			     (forward-char -1)
			     (c-backward-syntactic-ws lim)))
		       (back-to-indentation)
		       (looking-at c-class-key))))
	    (cond
	     ;; CASE 5C.1: non-hanging colon on an inher intro
	     ((= char-after-ip ?:)
	      (c-backward-syntactic-ws lim)
	      (c-add-syntax 'inher-intro (c-point 'boi))
	      ;; don't add inclass symbol since relative point already
	      ;; contains any class offset
	      )
	     ;; CASE 5C.2: hanging colon on an inher intro
	     ((= char-before-ip ?:)
	      (c-add-syntax 'inher-intro (c-point 'boi))
	      (and inclass-p (c-add-syntax 'inclass (aref inclass-p 0))))
	     ;; CASE agulbrahack.1: 
	     ((and inclass-p
		   c-access-key
		   (looking-at c-access-key))
	      (c-add-syntax 'access-label (c-point 'bonl))
	      )
	     ;; CASE 5C.3: a continued inheritance line
	     (t
	      (c-beginning-of-inheritance-list lim)
	      (c-add-syntax 'inher-cont (point))
	      ;; don't add inclass symbol since relative point already
	      ;; contains any class offset
	      )))
	   ;; CASE 5D: this could be a top-level compound statement or a
 	   ;; member init list continuation
	   ((= char-before-ip ?,)
	    (goto-char indent-point)
	    (c-backward-syntactic-ws lim)
	    (while (and (< lim (point))
			(= (preceding-char) ?,))
	      ;; this will catch member inits with multiple
	      ;; line arglists
	      (forward-char -1)
	      (c-backward-syntactic-ws (c-point 'bol))
	      (if (= (preceding-char) ?\))
		  (backward-sexp 1))
	      ;; now continue checking
	      (beginning-of-line)
	      (c-backward-syntactic-ws lim))
	    (cond
	     ;; CASE 5D.1: hanging member init colon, but watch out
	     ;; for bogus matches on access specifiers inside classes.
	     ((and (= (preceding-char) ?:)
		   (save-excursion
		     (forward-word -1)
		     (not (looking-at c-access-key))))
	      (goto-char indent-point)
	      (c-backward-syntactic-ws lim)
	      (c-safe (backward-sexp 1))
	      (c-add-syntax 'member-init-cont (c-point 'boi))
	      ;; we do not need to add class offset since relative
	      ;; point is the member init above us
	      )
	     ;; CASE 5D.2: non-hanging member init colon
	     ((progn
		(c-forward-syntactic-ws indent-point)
		(= (following-char) ?:))
	      (skip-chars-forward " \t:")
	      (c-add-syntax 'member-init-cont (point)))
	     ;; CASE 5D.3: perhaps a multiple inheritance line?
	     ((looking-at c-inher-key)
	      (c-add-syntax 'inher-cont (c-point 'boi)))
	     ;; CASE 5D.4: perhaps a template list continuation?
	     ((save-excursion
		(skip-chars-backward "^<" lim)
		;; not sure if this is the right test, but it should
		;; be fast and mostly accurate.
		(and (= (preceding-char) ?<)
		     (not (c-in-literal lim))))
	      ;; we can probably indent it just like and arglist-cont
	      (c-add-syntax 'arglist-cont (point)))
	     ;; CASE 5D.5: perhaps a top-level statement-cont
	     (t
	      (c-beginning-of-statement-1 lim)
	      ;; skip over any access-specifiers
	      (and inclass-p c-access-key
		   (while (looking-at c-access-key)
		     (forward-line 1)))
	      ;; skip over comments, whitespace
	      (c-forward-syntactic-ws indent-point)
	      (c-add-syntax 'statement-cont (c-point 'boi)))
	     ))
	   ;; CASE 5E: we are looking at a access specifier
	   ((and inclass-p
		 c-access-key
		 (looking-at c-access-key))
	    (c-add-syntax 'access-label (c-point 'bonl))
	    (c-add-syntax 'inclass (aref inclass-p 0)))
	   ;; CASE 5F: we are looking at the brace which closes the
	   ;; enclosing nested class decl
	   ((and inclass-p
		 (= char-after-ip ?})
		 (save-excursion
		   (save-restriction
		     (widen)
		     (forward-char 1)
		     (and
		      (condition-case nil
			  (progn (backward-sexp 1) t)
			(error nil))
		      (= (point) (aref inclass-p 1))
		      ))))
	    (save-restriction
	      (widen)
	      (goto-char (aref inclass-p 0))
	      (c-add-syntax 'class-close (c-point 'boi))))
	   ;; CASE 5G: we could be looking at subsequent knr-argdecls
	   ((and c-recognize-knr-p
		 (save-excursion
		   (c-backward-syntactic-ws lim)
		   (while (memq (preceding-char) '(?\; ?,))
		     (beginning-of-line)
		     (setq placeholder (point))
		     (c-backward-syntactic-ws lim))
		   (and (= (preceding-char) ?\))
			(or (not (eq major-mode 'objc-mode))
			    (progn
			      (forward-sexp -1)
			      (forward-char -1)
			      (c-backward-syntactic-ws)
			      (not (or (= (preceding-char) ?-)
				       (= (preceding-char) ?+)
				       ;; or a class category
				       (progn
					 (forward-sexp -2)
					 (looking-at c-class-key))
				       )))))
		   )
		 (save-excursion
		   (c-beginning-of-statement-1)
		   (not (looking-at "typedef[ \t\n]+"))))
	    (goto-char placeholder)
	    (c-add-syntax 'knr-argdecl (c-point 'boi)))
	   ;; CASE 5H: we are at the topmost level, make sure we skip
	   ;; back past any access specifiers
	   ((progn
	      (c-backward-syntactic-ws lim)
	      (while (and inclass-p
			  c-access-key
			  (not (bobp))
			  (save-excursion
			    (c-safe (progn (backward-sexp 1) t))
			    ;; agulbrahack 2
			    (and (looking-at "slots:")
				 (backward-sexp 1))
			    (looking-at c-access-key)))
		(backward-sexp 1)
		(c-backward-syntactic-ws lim))
	      (or (bobp)
		  (memq (preceding-char) '(?\; ?\}))))
	    ;; real beginning-of-line could be narrowed out due to
	    ;; enclosure in a class block
	    (save-restriction
	      (widen)
	      (c-add-syntax 'topmost-intro (c-point 'bol))
	      (if inclass-p
		  (progn
		    (goto-char (aref inclass-p 1))
		    (c-add-syntax 'inclass (c-point 'boi))))))
	   ;; CASE 5I: we are at a method definition continuation line
	   ((and (eq major-mode 'objc-mode)
		 (progn
		   (c-beginning-of-statement-1 lim)
		   (beginning-of-line)
		   (looking-at c-ObjC-method-key)))
	    (c-add-syntax 'objc-method-args-cont (point)))
	   ;; CASE 5J: we are at a topmost continuation line
	   (t
	    (c-beginning-of-statement-1 lim)
	    (c-forward-syntactic-ws)
	    (c-add-syntax 'topmost-intro-cont (c-point 'boi)))
	   ))				; end CASE 5
	 ;; CASE 6: line is an expression, not a statement.  Most
	 ;; likely we are either in a function prototype or a function
	 ;; call argument list
	 ((/= (char-after containing-sexp) ?{)
	  (c-backward-syntactic-ws containing-sexp)
	  (cond
	   ;; CASE 6A: we are looking at the arglist closing paren or
	   ;; at an Objective-C method call closing bracket.
	   ((and (/= char-before-ip ?,)
		 (memq char-after-ip '(?\) ?\])))
	    (if (and (eq major-mode 'objc-mode)
		     (progn
		       (goto-char (1- containing-sexp))
		       (c-backward-syntactic-ws lim)
		       (not (looking-at c-symbol-key))))
		(c-add-syntax 'statement-cont containing-sexp)
	      (goto-char containing-sexp)
	      (c-add-syntax 'arglist-close (c-point 'boi))))
	   ;; CASE 6B: we are looking at the first argument in an empty
	   ;; argument list. Use arglist-close if we're actually
	   ;; looking at a close paren or bracket.
	   ((memq char-before-ip '(?\( ?\[))
	    (goto-char containing-sexp)
	    (c-add-syntax 'arglist-intro (c-point 'boi)))
	   ;; CASE 6C: we are inside a conditional test clause. treat
	   ;; these things as statements
	   ((save-excursion
	     (goto-char containing-sexp)
	     (and (c-safe (progn (forward-sexp -1) t))
		  (looking-at "\\<for\\>")))
	    (goto-char (1+ containing-sexp))
	    (c-forward-syntactic-ws indent-point)
	    (c-beginning-of-statement-1 containing-sexp)
	    (if (= char-before-ip ?\;)
		(c-add-syntax 'statement (point))
	      (c-add-syntax 'statement-cont (point))
	      ))
	   ;; CASE 6D: maybe a continued method call. This is the case
	   ;; when we are inside a [] bracketed exp, and what precede
	   ;; the opening bracket is not an identifier.
	   ((and (eq major-mode 'objc-mode)
		 (= (char-after containing-sexp) ?\[)
		 (save-excursion
		   (goto-char (1- containing-sexp))
		   (c-backward-syntactic-ws (c-point 'bod))
		   (if (not (looking-at c-symbol-key))
		       (c-add-syntax 'objc-method-call-cont containing-sexp))
		   )))
	   ;; CASE 6E: we are looking at an arglist continuation line,
	   ;; but the preceding argument is on the same line as the
	   ;; opening paren.  This case includes multi-line
	   ;; mathematical paren groupings, but we could be on a
	   ;; for-list continuation line
	   ((and (save-excursion
		   (goto-char (1+ containing-sexp))
		   (skip-chars-forward " \t")
		   (not (eolp)))
		 (save-excursion
		   (c-beginning-of-statement-1 lim)
		   (skip-chars-backward " \t([")
		   (<= (point) containing-sexp)))
	    (goto-char containing-sexp)
	    (c-add-syntax 'arglist-cont-nonempty (c-point 'boi)))
	   ;; CASE 6F: we are looking at just a normal arglist
	   ;; continuation line
	   (t (c-beginning-of-statement-1 containing-sexp)
	      (forward-char 1)
	      (c-forward-syntactic-ws indent-point)
	      (c-add-syntax 'arglist-cont (c-point 'boi)))
	   ))
	 ;; CASE 7: func-local multi-inheritance line
	 ((and c-baseclass-key
	       (save-excursion
		 (goto-char indent-point)
		 (skip-chars-forward " \t")
		 (looking-at c-baseclass-key)))
	  (goto-char indent-point)
	  (skip-chars-forward " \t")
	  (cond
	   ;; CASE 7A: non-hanging colon on an inher intro
	   ((= char-after-ip ?:)
	    (c-backward-syntactic-ws lim)
	    (c-add-syntax 'inher-intro (c-point 'boi)))
	   ;; CASE 7B: hanging colon on an inher intro
	   ((= char-before-ip ?:)
	    (c-add-syntax 'inher-intro (c-point 'boi)))
	   ;; CASE 7C: a continued inheritance line
	   (t
	    (c-beginning-of-inheritance-list lim)
	    (c-add-syntax 'inher-cont (point))
	    )))
	 ;; CASE 8: we are inside a brace-list
	 ((setq placeholder (c-inside-bracelist-p containing-sexp state))
	  (cond
	   ;; CASE 8A: brace-list-close brace
	   ((and (= char-after-ip ?})
		 (c-safe (progn (forward-char 1)
				(backward-sexp 1)
				t))
		 (= (point) containing-sexp))
	    (c-add-syntax 'brace-list-close (c-point 'boi)))
	   ;; CASE 8B: we're looking at the first line in a brace-list
	   ((save-excursion
	      (goto-char indent-point)
	      (c-backward-syntactic-ws containing-sexp)
	      (= (point) (1+ containing-sexp)))
	    (goto-char containing-sexp)
	    ;;(if (= char-after-ip ?{)
		;;(c-add-syntax 'brace-list-open (c-point 'boi))
	    (c-add-syntax 'brace-list-intro (c-point 'boi))
	    )
	    ;;))			; end CASE 8B
	   ;; CASE 8C: this is just a later brace-list-entry
	   (t (goto-char (1+ containing-sexp))
	      (c-forward-syntactic-ws indent-point)
	      (if (= char-after-ip ?{)
		  (c-add-syntax 'brace-list-open (point))
		(c-add-syntax 'brace-list-entry (point))
		))			; end CASE 8C
	   ))				; end CASE 8
	 ;; CASE 9: A continued statement
	 ((and (not (memq char-before-ip '(?\; ?} ?:)))
	       (> (point)
		  (save-excursion
		    (c-beginning-of-statement-1 containing-sexp)
		    (setq placeholder (point))))
	       (/= placeholder containing-sexp))
	  (goto-char indent-point)
	  (skip-chars-forward " \t")
	  (let ((after-cond-placeholder
		 (save-excursion
		   (goto-char placeholder)
		   (if (looking-at c-conditional-key)
		       (progn
			 (c-safe (c-skip-conditional))
			 (c-forward-syntactic-ws)
			 (if (memq (following-char) '(?\;))
			     (progn
			       (forward-char 1)
			       (c-forward-syntactic-ws)))
			 (point))
		     nil))))
	    (cond
	     ;; CASE 9A: substatement
	     ((and after-cond-placeholder
		   (>= after-cond-placeholder indent-point))
	      (goto-char placeholder)
	      (if (= char-after-ip ?{)
		  (c-add-syntax 'substatement-open (c-point 'boi))
		(c-add-syntax 'substatement (c-point 'boi))))
	     ;; CASE 9B: open braces for class or brace-lists
	     ((= char-after-ip ?{)
	      (cond
	       ;; CASE 9B.1: class-open
	       ((save-excursion
		  (goto-char indent-point)
		  (skip-chars-forward " \t{")
		  (let ((decl (c-search-uplist-for-classkey (c-parse-state))))
		    (and decl
			 (setq placeholder (aref decl 0)))
		    ))
		(c-add-syntax 'class-open placeholder))
	       ;; CASE 9B.2: brace-list-open
	       ((or (save-excursion
		      (goto-char placeholder)
		      (looking-at "\\<enum\\>"))
		    (= char-before-ip ?=))
		(c-add-syntax 'brace-list-open placeholder))
	       ;; CASE 9B.3: catch-all for unknown construct.
	       (t
		;; Even though this isn't right, it's the best I'm
		;; going to do for now. Exceptions probably fall
		;; through to here, but aren't supported yet.  Also,
		;; after the next release, I may call a recognition
		;; hook like so: (run-hooks 'c-recognize-hook), but I
		;; dunno.
		(goto-char placeholder)
		(c-add-syntax 'statement-cont (c-point 'boi))
		(c-add-syntax 'block-open))
	       ))
	     ;; CASE 9C: iostream insertion or extraction operator
	     ((looking-at "<<\\|>>")
	      (goto-char placeholder)
	      (and after-cond-placeholder
		   (goto-char after-cond-placeholder))
	      (while (and (re-search-forward "<<\\|>>" indent-point 'move)
			  (c-in-literal placeholder)))
	      ;; if we ended up at indent-point, then the first
	      ;; streamop is on a separate line. Indent the line like
	      ;; a statement-cont instead
	      (if (/= (point) indent-point)
		  (c-add-syntax 'stream-op (c-point 'boi))
		(c-backward-syntactic-ws lim)
		(c-add-syntax 'statement-cont (c-point 'boi))))
	     ;; CASE 9D: continued statement. find the accurate
	     ;; beginning of statement or substatement
	     (t
	      (c-beginning-of-statement-1 after-cond-placeholder)
	      ;; KLUDGE ALERT!  c-beginning-of-statement-1 can leave
	      ;; us before the lim we're passing in.  It should be
	      ;; fixed, but I'm worried about side-effects at this
	      ;; late date.  Fix for v5.
	      (goto-char (or (and after-cond-placeholder
				  (max after-cond-placeholder (point)))
			     (point)))
	      (c-add-syntax 'statement-cont (point)))
	     )))
	 ;; CASE 10: an else clause?
	 ((looking-at "\\<else\\>[^_]")
	  (c-backward-to-start-of-if containing-sexp)
	  (c-add-syntax 'else-clause (c-point 'boi)))
	 ;; CASE 11: Statement. But what kind?  Lets see if its a
	 ;; while closure of a do/while construct
	 ((progn
	    (goto-char indent-point)
	    (skip-chars-forward " \t")
	    (and (looking-at "while\\b[^_]")
		 (save-excursion
		   (c-backward-to-start-of-do containing-sexp)
		   (setq placeholder (point))
		   (looking-at "do\\b[^_]"))
		 ))
	  (c-add-syntax 'do-while-closure placeholder))
	 ;; CASE 12: A case or default label
	 ((looking-at c-switch-label-key)
	  (goto-char containing-sexp)
	  ;; check for hanging braces
	  (if (/= (point) (c-point 'boi))
	      (forward-sexp -1))
	  (c-add-syntax 'case-label (c-point 'boi)))
	 ;; CASE 13: any other label
	 ((looking-at c-label-key)
	  (goto-char containing-sexp)
	  (c-add-syntax 'label (c-point 'boi)))
	 ;; CASE 14: block close brace, possibly closing the defun or
	 ;; the class
	 ((= char-after-ip ?})
	  (let* ((lim (c-safe-position containing-sexp fullstate))
		 (relpos (save-excursion
			   (goto-char containing-sexp)
			   (if (/= (point) (c-point 'boi))
			       (c-beginning-of-statement-1 lim))
			   (c-point 'boi))))
	    (cond
	     ;; CASE 14A: does this close an inline?
	     ((progn
		(goto-char containing-sexp)
		(c-search-uplist-for-classkey state))
	      (c-add-syntax 'inline-close relpos))
	     ;; CASE 14B: if there an enclosing brace that hasn't
	     ;; been narrowed out by a class, then this is a
	     ;; block-close
	     ((c-most-enclosing-brace state)
	      (c-add-syntax 'block-close relpos))
	     ;; CASE 14C: find out whether we're closing a top-level
	     ;; class or a defun
	     (t
	      (save-restriction
		(narrow-to-region (point-min) indent-point)
		(let ((decl (c-search-uplist-for-classkey (c-parse-state))))
		  (if decl
		      (c-add-syntax 'class-close (aref decl 0))
		    (c-add-syntax 'defun-close relpos)))))
	     )))
	 ;; CASE 15: statement catchall
	 (t
	  ;; we know its a statement, but we need to find out if it is
	  ;; the first statement in a block
	  (goto-char containing-sexp)
	  (forward-char 1)
	  (c-forward-syntactic-ws indent-point)
	  ;; now skip forward past any case/default clauses we might find.
	  (while (or (c-skip-case-statement-forward fullstate indent-point)
		     (and (looking-at c-switch-label-key)
			  (not inswitch-p)))
	    (setq inswitch-p t))
	  ;; we want to ignore non-case labels when skipping forward
	  (while (and (looking-at c-label-key)
		      (goto-char (match-end 0)))
	    (c-forward-syntactic-ws indent-point))
	  (cond
	   ;; CASE 15A: we are inside a case/default clause inside a
	   ;; switch statement.  find out if we are at the statement
	   ;; just after the case/default label.
	   ((and inswitch-p
		 (progn
		   (goto-char indent-point)
		   (c-backward-syntactic-ws containing-sexp)
		   (back-to-indentation)
		   (setq placeholder (point))
		   (looking-at c-switch-label-key)))
	    (goto-char indent-point)
	    (skip-chars-forward " \t")
	    (if (= (following-char) ?{)
		(c-add-syntax 'statement-case-open placeholder)
	      (c-add-syntax 'statement-case-intro placeholder)))
	   ;; CASE 15B: continued statement
	   ((= char-before-ip ?,)
	    (c-add-syntax 'statement-cont (c-point 'boi)))
	   ;; CASE 15C: a question/colon construct?  But make sure
	   ;; what came before was not a label, and what comes after
	   ;; is not a globally scoped function call!
	   ((or (and (memq char-before-ip '(?: ??))
		     (save-excursion
		       (goto-char indent-point)
		       (c-backward-syntactic-ws lim)
		       (back-to-indentation)
		       (not (looking-at c-label-key))))
		(and (memq char-after-ip '(?: ??))
		     (save-excursion
		       (goto-char indent-point)
		       (skip-chars-forward " \t")
		       ;; watch out for scope operator
		       (not (looking-at "::")))))
	    (c-add-syntax 'statement-cont (c-point 'boi)))
	   ;; CASE 15D: any old statement
	   ((< (point) indent-point)
	    (let ((safepos (c-most-enclosing-brace fullstate)))
	      (goto-char indent-point)
	      (c-beginning-of-statement-1 safepos)
	      (c-add-syntax 'statement (c-point 'boi))
	      (if (= char-after-ip ?{)
		  (c-add-syntax 'block-open))))
	   ;; CASE 15E: first statement in an inline, or first
	   ;; statement in a top-level defun. we can tell this is it
	   ;; if there are no enclosing braces that haven't been
	   ;; narrowed out by a class (i.e. don't use bod here!)
	   ((save-excursion
	      (save-restriction
		(widen)
		(goto-char containing-sexp)
		(c-narrow-out-enclosing-class state containing-sexp)
		(not (c-most-enclosing-brace state))))
	    (goto-char containing-sexp)
	    ;; if not at boi, then defun-opening braces are hung on
	    ;; right side, so we need a different relpos
	    (if (/= (point) (c-point 'boi))
		(progn
		  (c-backward-syntactic-ws)
		  (c-safe (forward-sexp (if (= (preceding-char) ?\))
					    -1 -2)))
		  ))
	    (c-add-syntax 'defun-block-intro (c-point 'boi)))
	   ;; CASE 15F: first statement in a block
	   (t (goto-char containing-sexp)
	      (if (/= (point) (c-point 'boi))
		  (c-beginning-of-statement-1
		   (if (= (point) lim)
		       (c-safe-position (point) state) lim)))
	      (c-add-syntax 'statement-block-intro (c-point 'boi))
	      (if (= char-after-ip ?{)
		  (c-add-syntax 'block-open)))
	   ))
	 )

	;; now we need to look at any modifiers
	(goto-char indent-point)
	(skip-chars-forward " \t")
	;; are we looking at a comment only line?
	(if (looking-at c-comment-start-regexp)
	    (c-add-syntax 'comment-intro))
	;; we might want to give additional offset to friends (in C++).
	(if (and (eq major-mode 'c++-mode)
		 (looking-at c-C++-friend-key))
	    (c-add-syntax 'friend))
	;; return the syntax
	syntax))))
