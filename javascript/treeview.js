// ---------------------------------------------
// --- Name:    Easy DHTML Treeview           --
// --- Author:  D.D. de Kerf                  --
// --- Adapted: Dieter Hametner		      --
// --- Version: 0.2          Date: 13-6-2001  --
// ---------------------------------------------
function Toggle(node)
{
	// Unfold the branch if it isn't visible
	if (node.nextSibling.style.display == 'none')
	{
		// Change the image (if there is an image)
		if (node.childNodes.length > 0)
		{
			if (node.childNodes.item(0).nodeName == "IMG")
			{
				node.childNodes.item(0).src = "minus.png";
			}
			if (node.childNodes.item(1).nodeName == "IMG")
			{
				node.childNodes.item(1).src = "folder_open.png";
			}
		}

		node.nextSibling.style.display = 'block';
	}
	// Collapse the branch if it IS visible
	else
	{
		// Change the image (if there is an image)
		if (node.childNodes.length > 0)
		{
			if (node.childNodes.item(0).nodeName == "IMG")
			{
				node.childNodes.item(0).src = "plus.png";
			}
			if (node.childNodes.item(1).nodeName == "IMG")
			{
				node.childNodes.item(1).src = "folder_closed.png";
			}
		}

		node.nextSibling.style.display = 'none';
	}

}
