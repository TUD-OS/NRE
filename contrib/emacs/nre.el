;;; -*- Mode: Emacs-Lisp -*-

;;; Copyright (C) 2012, Julian Stecklina <jsteckli@os.inf.tu-dresden.de>
;;; Economic rights: Technische Universitaet Dresden (Germany)
;;;
;;; This file is part of NRE (NOVA runtime environment).
;;;
;;; NRE is free software: you can redistribute it and/or modify
;;; it under the terms of the GNU General Public License version 2 as
;;; published by the Free Software Foundation.
;;;
;;; NRE is distributed in the hope that it will be useful, but
;;; WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
;;; General Public License version 2 for more details.

;;; To globally install this, add it to your Emacs init file:
;;; (add-to-list 'load-path "~/src/path-to-nre/contrib/emacs/")
;;; (nre-setup)

(defvar nre-default-tab-width 4)

(defun nre-install-style ()
  "Enable NRE minor mode, if the current buffer looks like part of the NRE tree."
  (interactive)
  (when (string-match "/nre/" buffer-file-name)
    ;; Set C/C++ indentation style
    (when (member major-mode '(c++-mode c-mode))
      (c-set-style "stroustrup")
      (setq indent-tabs-mode nil
            tab-width nre-default-tab-width)
      (c-set-offset 'case-label '+))))


(defun nre-setup ()
  "Registers NRE hooks."
  (interactive)
  (add-hook 'c-mode-common-hook 'nre-install-style))

(provide 'nre)

;;; EOF
