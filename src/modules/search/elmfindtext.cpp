/**************************************************************************
 *  This file is part of QXmlEdit                                         *
 *  Copyright (C) 2015 by Luca Bellonda and individual contributors       *
 *    as indicated in the AUTHORS file                                    *
 *  lbellonda _at_ gmail.com                                              *
 *                                                                        *
 * This library is free software; you can redistribute it and/or          *
 * modify it under the terms of the GNU Library General Public            *
 * License as published by the Free Software Foundation; either           *
 * version 2 of the License, or (at your option) any later version.       *
 *                                                                        *
 * This library is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 * Library General Public License for more details.                       *
 *                                                                        *
 * You should have received a copy of the GNU Library General Public      *
 * License along with this library; if not, write to the                  *
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,       *
 * Boston, MA  02110-1301  USA                                            *
 **************************************************************************/

#include "regoladefinitions.h"
#include "undo/elupdateelementcommand.h"


Element * Regola::findText(QTreeWidget *tree, FindTextParams &findArgs, Element *selectedItem)
{
    if(findArgs.useXQuery()) {
        searchWithXQuery(findArgs, selectedItem);
        return NULL;
    }
    unhiliteAll();
    if(findArgs.isCloseUnrelated()) {

    }
    if(!findArgs.isFindAllOccurrences()) {
        return findNextTextMatch(tree, findArgs, selectedItem);
    }
    if((NULL != selectedItem) && findArgs.isLookOnlyChildren()) {
        if(findArgs.isCloseUnrelated() && (NULL != selectedItem->getUI())) {
            if(selectedItem->getUI()->isExpanded()) {
                selectedItem->getUI()->setExpanded(false);
            }
        }
        selectedItem->findText(findArgs);
    } else {
        bool isHiliteAll = findArgs.isHiliteAll();
        QVectorIterator<Element*> it(childItems);
        while(it.hasNext()) {
            Element* element = it.next();
            if(findArgs.isCloseUnrelated() && (NULL != element->getUI())) {
                if(element->getUI()->isExpanded()) {
                    element->getUI()->setExpanded(false);
                }
            }
            if(element->findText(findArgs)) {
                if(!isHiliteAll) {
                    break;
                }
            }
        }// while next
    }
    return NULL ;
}

/*
void Regola::findNextText(FindTextParams &findArgs, Element *selectedItem)
{
    if(findArgs.useXQuery()) {
        searchWithXQuery(findArgs, selectedItem);
        return ;
    }
    unhiliteAll();
    if(findArgs.isCloseUnrelated()) {

    }
    if((NULL != selectedItem) && findArgs.isLookOnlyChildren()) {
        if(findArgs.isCloseUnrelated() && (NULL != selectedItem->getUI())) {
            if(selectedItem->getUI()->isExpanded()) {
                selectedItem->getUI()->setExpanded(false);
            }
        }
        selectedItem->findText(findArgs);
    } else {
        bool isHiliteAll = findArgs.isHiliteAll();
        QVectorIterator<Element*> it(childItems);
        while(it.hasNext()) {
            Element* element = it.next();
            if(findArgs.isCloseUnrelated() && (NULL != element->getUI())) {
                if(element->getUI()->isExpanded()) {
                    element->getUI()->setExpanded(false);
                }
            }
            if(element->findText(findArgs)) {
                if(!isHiliteAll) {
                    break;
                }
            }
        }// while next
    }
}
*/

// nuovo algoritmo: ambito di ricerca
bool Element::searchInScope(FindTextParams &findArgs)
{
    Utils::TODO_THIS_RELEASE("Remove the commented code above");
    if(!findArgs.isSearchWithScope()) {
        return true;
    }
    QString scope = findArgs.mainScope();
    if(scope.isEmpty() || (tag() == scope)) {
        QStringList scopes = findArgs.getScopes(); //get a reference!
        int scopeCount = scopes.count();
        Element *el = this ;
        for(int i = scopeCount - 1 ; i >= 0 ; i--) {
            el = el->parentElement ;
            if(NULL == el) {
                return false;
            }
            QString thisScope = scopes.at(i) ;
            if(!thisScope.isEmpty() && (el->tag() != thisScope)) {
                return false;
            }
        }
        return true;
    }
    return false ;
}


/*!
 * \brief Element::matchText
 * \param findArgs
 * \return true if there is a match on the element or direct owned elements: text and/or attributes
 */
bool Element::matchText(FindTextParams &findArgs)
{
    bool isFound = false;
    FindTextParams::EFindTarget findTarget = findArgs.getFindTarget();

    switch(type) {
    default:
    case ET_ELEMENT: {

        if(searchInScope(findArgs)) {
            if((FindTextParams::FIND_ALL == findTarget) || (FindTextParams::FIND_TAG == findTarget)) {
                if(findArgs.isTextMatched(tag()))
                    isFound = true ;
            }
            if(!isFound) {
                if((FindTextParams::FIND_ALL == findTarget) || (FindTextParams::FIND_TEXT == findTarget)) {
                    foreach(TextChunk * chunk, textNodes) {
                        if(findArgs.isTextMatched(chunk->text)) {
                            isFound = true ;
                        }
                    }
                } else if(FindTextParams::FIND_TEXT_BASE64) {
                    foreach(TextChunk * chunk, textNodes) {
                        if(findArgs.isTextBase64Matched(chunk->text)) {
                            isFound = true ;
                        }
                    }
                }
            }
            if(!isFound) {
                QVectorIterator<Attribute*>  attrs(attributes);
                while(attrs.hasNext()) {
                    Attribute* a = attrs.next();
                    if((FindTextParams::FIND_ALL == findTarget) || (FindTextParams::FIND_ATTRIBUTE_NAME == findTarget)) {
                        if(findArgs.isTextMatched(a->name)) {
                            isFound = true ;
                        }
                    }
                    if(!isFound) {
                        if((FindTextParams::FIND_ALL == findTarget) || (FindTextParams::FIND_ATTRIBUTE_VALUE == findTarget)) {
                            if(findArgs.isSearchInAttribute()) {
                                if((a->name == findArgs.attributeName())
                                        && (FindTextParams::FIND_ATTRIBUTE_VALUE == findTarget)) {
                                    isFound = true ;
                                }
                            } else if(findArgs.isTextMatched(a->value)) {
                                isFound = true ;
                            }
                        } else if(FindTextParams::FIND_TEXT_BASE64) {
                            if(findArgs.isTextBase64Matched(a->value)) {
                                isFound = true ;
                            }
                        }
                    } // if ! found
                } // for attrs
            } //! found
        }
    }
    break;
    case ET_TEXT:
        if((FindTextParams::FIND_ALL == findTarget) || (FindTextParams::FIND_TEXT == findTarget)) {
            if(findArgs.isTextMatched(text)) {
                isFound = true ;
            }
        } else if(FindTextParams::FIND_TEXT_BASE64) {
            if(findArgs.isTextBase64Matched(text)) {
                isFound = true ;
            }
        }
        break;
    case ET_PROCESSING_INSTRUCTION: {
        if(FindTextParams::FIND_ALL == findTarget) {
            if(findArgs.isTextMatched(getPITarget()) || findArgs.isTextMatched(getPIData())) {
                isFound = true ;
            }
        }
    }
    break;
    case ET_COMMENT: {
        if((FindTextParams::FIND_ALL == findTarget) || (FindTextParams::FIND_COMMENT == findTarget)) {
            if(findArgs.isTextMatched(getComment())) {
                isFound = true ;
            }
        }
    }
    break;
    }
    return isFound ;
}


bool Element::findText(FindTextParams &findArgs)
{
    bool isFound = false;
    bool isFoundInChild = false;
    bool isHiliteAll = findArgs.isHiliteAll();
    bool isCloseUnrelated = findArgs.isCloseUnrelated();
    if(findArgs.isCountingOnly()) {
        isCloseUnrelated = false;
    }

    isFound = matchText(findArgs);
    if(isFound) {
        findArgs.newOccurence(selfInfo.totalSize + childrenInfo.totalSize);
        if(!findArgs.isCountingOnly()) {
            findArgs.addSelection(this);
            hilite();
            if(findArgs.isSelToBookmarks()) {
                parentRule->addBookmark(this);
            }
        }
    }
    //append children
    if(isHiliteAll) {
        foreach(Element * value, childItems) {
            if(value->findText(findArgs)) {
                isFoundInChild = true ;
            }
        }
    }

    bool isFoundSomeWhere = isFoundInChild || isFound ;
    if(!findArgs.isCountingOnly()) {
        if(isCloseUnrelated && (NULL != ui)) {
            if(isFoundInChild) {
                if(!ui->isExpanded()) {
                    ui->setExpanded(true);
                }
            } else {
                if(ui->isExpanded()) {
                    if(ui->isExpanded()) {
                        ui->setExpanded(false);
                    }
                }
            }
        }
    }
    return isFoundSomeWhere  ;
}


Element *Regola::selectElement(QTreeWidget *tree, Element * selectedElement)
{
    if(NULL != tree) {
        QTreeWidgetItem *item = NULL ;
        if(NULL != selectedElement) {
            item = selectedElement->getUI();
            tree->setCurrentItem(item, 0);
            tree-> scrollToItem(item, QAbstractItemView::PositionAtTop);
        }
    }
    return selectedElement ;
}

Element *Regola::findNextTextMatch(QTreeWidget *tree, FindTextParams &findArgs, Element *selectedItem)
{
    return selectElement(tree, findTheNextTextMatch(findArgs, selectedItem));
}

Element *Regola::findTheNextTextMatch(FindTextParams &findArgs, Element *selectedItem)
{
    bool hasWrapAround = false;
    bool isNext = findArgs.isFindNext();
    bool skipFirst = false;
    Element *startElement = selectedItem ;
    if(NULL == startElement) {
        startElement = isNext ? firstChild() : lastChild() ;
        skipFirst = true ;
    }
    if(NULL == startElement) {
        return NULL ;
    }

    Element *element = startElement;
    while(true) {
        if(!skipFirst) {
            Element *child ;
            if(isNext) {
                child = element->firstChild();
            } else {
                child = element->lastChild();
            }
            if(child != NULL) {
                element = child ;
            } else {
                // no more children to test, resorting to siblings.
                Element *nextElement = NULL ;
                do {
                    // move to the next/previous object
                    nextElement = isNext ? element->nextSibling() : element->previousSibling();


                    /*Element *parent = element->parent();
                    int currentChildIndex = element->indexOfSelfAsChild();
                    if( NULL == parent ) {
                        // this is the root
                        nextElement = root->child(currentChildIndex+indexIncrement);
                    } else {
                        nextElement = parent->child(currentChildIndex+indexIncrement);
                    }*/
                    if(NULL == nextElement) {
                        element = element->parent();
                        if(NULL == element) {
                            // this is the top level, no futher move without wraparound
                            if(findArgs.isWrapAround()) {
                                nextElement = isNext ? firstChild() : lastChild() ;
                                if(hasWrapAround) {
                                    // point already touched
                                    return NULL ;
                                }
                                hasWrapAround = true ;
                            } else {
                                // no match
                                return NULL;
                            }
                        } // if top
                    } // if null next
                } while(NULL == nextElement);
                element = nextElement ;
            }
            // if the next element is the same from which we startd, no more processing is possible
            // a complete wraparound was done
            if(element == startElement) {
                return NULL ;
            }
        } else {
            skipFirst = false;
        }
        bool found = element->matchText(findArgs);
        if(found) {
            return element ;
        }
    } // loop on next element
    return NULL ;
}
