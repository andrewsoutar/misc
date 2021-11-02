;;; http://cslibrary.stanford.edu/109/TreeListRecursion.html

(defstruct node
  (value nil)
  (left nil :type (or null node))
  (right nil :type (or null node)))

(defun tree-list (tree)
  (labels ((connect (left right)
             (setf (node-right left) right
                   (node-left right) left))
           (rec (subtree &aux temp (left-edge subtree) (right-edge subtree))
             (when (node-left subtree)
               (setf (values left-edge temp) (rec (node-left subtree)))
               (connect temp subtree))
             (when (node-right subtree)
               (setf (values temp right-edge) (rec (node-right subtree)))
               (connect subtree temp))
             (values left-edge right-edge)))
    (multiple-value-bind (leftmost rightmost) (rec tree)
      (connect rightmost leftmost)
      leftmost)))
